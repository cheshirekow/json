// Copyright 2018 Josh Bialkowski <josh.bialkowski@gmail.com>
#include <cxxabi.h>
#include <execinfo.h>
#include <algorithm>

#include "argue/argue.h"

namespace argue {

// =============================================================================
//                                 Utilities
// =============================================================================

std::string ToUpper(const std::string& str) {
  std::string out = str;
  for (char& c : out) {
    if ('a' <= c && c <= 'z') {
      c = 'A' + (c - 'a');
    }
  }
  return std::move(out);
}

std::string ToLower(const std::string& str) {
  // http://en.cppreference.com/w/cpp/string/byte/tolower
  // std::transform(s.begin(), s.end(), s.begin(),
  //                [](unsigned char c) { return std::tolower(c); });

  std::string out = str;
  for (char& c : out) {
    if ('A' <= c && c <= 'Z') {
      c = 'a' + (c - 'A');
    }
  }
  return std::move(out);
}

// =============================================================================
//                    Exception Handling and Stack Traces
// =============================================================================

// Parse the textual content of a stacktrace into their individual components.
// A line within a stacktrace has format:
// (<name>+<offset>) [<address>]
static void ParseTraceLine(char* symbol, TraceLine* traceline) {
  char* begin_name = 0;
  char* begin_offset = 0;
  char* end_offset = 0;
  char* begin_addr = 0;
  char* end_addr = 0;

  // Find parentheses and +address offset surrounding the mangled name:
  // ./module(function+0x15c) [0x8048a6d]
  for (char* ptr = symbol; *ptr; ++ptr) {
    if (*ptr == '(') {
      begin_name = ptr;
    } else if (*ptr == '+') {
      begin_offset = ptr;
    } else if (*ptr == ')' && begin_offset) {
      end_offset = ptr;
    } else if (*ptr == '[' && end_offset) {
      begin_addr = ptr;
    } else if (*ptr == ']' && begin_addr) {
      end_addr = ptr;
    }
  }

  if (begin_name) {
    *begin_name = '\0';
  }
  traceline->file = symbol;

  if (begin_offset) {
    *begin_offset = '\0';
  }
  if (end_offset) {
    *end_offset = '\0';
  }

  if (begin_offset && begin_name < begin_offset) {
    traceline->name = (begin_name + 1);
    traceline->offset = (begin_offset + 1);
  }

  if (end_addr) {
    *end_addr = '\0';
    traceline->saddr = (begin_addr + 1);
  }
}

// TODO(josh): use libunwind
StackTrace GetStacktrace(size_t skip_frames, size_t max_frames) {
  StackTrace result;
  std::vector<void*> addrlist(max_frames + 1);

  // http://man7.org/linux/man-pages/man3/backtrace.3.html
  int addrlen = backtrace(&addrlist[0], addrlist.size());
  if (addrlen == 0) {
    return {{0, "<empty, possibly corrupt>"}};
  }

  // symbol -> "filename(function+offset)" (this array needs to free())
  char** symbols = backtrace_symbols(&addrlist[0], addrlen);

  // We have to malloc this, can't use std::string(), because demangle function
  // may realloc() it.
  size_t funcnamesize = 256;
  char* funcnamebuf = static_cast<char*>(malloc(funcnamesize));

  // Iterate over the returned symbol lines, skipping frames as requested,
  // and one extra for this function.
  result.reserve(addrlen - skip_frames);
  for (size_t idx = skip_frames + 1; idx < static_cast<size_t>(addrlen);
       idx++) {
    TraceLine traceline{.addr = addrlist[idx]};
    ParseTraceLine(symbols[idx], &traceline);
    int status = 0;
    if (traceline.name.size()) {
      char* ret = abi::__cxa_demangle(traceline.name.c_str(), funcnamebuf,
                                      &funcnamesize, &status);
      if (status == 0) {
        funcnamebuf = ret;
        traceline.name = funcnamebuf;
      }
    }

    result.push_back(traceline);
  }

  free(funcnamebuf);
  free(symbols);
  return result;
}

const char* Exception::what() const noexcept {
  return this->message.c_str();
}

const char* Exception::ToString(TypeNo typeno) {
  switch (typeno) {
    case BUG:
      return "BUG";
    case CONFIG_ERROR:
      return "CONFIG_ERROR";
    case INPUT_ERROR:
      return "INPUT_ERROR";
    default:
      return "<invalid>";
  }
}

// message will be appended
Assertion::Assertion(Exception::TypeNo typeno, bool expr)
    : typeno(typeno), expr_(expr) {}

// construct with message
Assertion::Assertion(Exception::TypeNo typeno, bool expr,
                     const std::string& message)
    : typeno(typeno), expr_(expr) {
  if (!expr) {
    sstream << message;
  }
}

void operator&&(const argue::AssertionSentinel& sentinel,
                const argue::Assertion& assertion) {
  if (!assertion.expr_) {
    argue::Exception ex{assertion.typeno, assertion.sstream.str()};
    ex.file = sentinel.file;
    ex.lineno = sentinel.lineno;
    ex.stack_trace = GetStacktrace();
    throw ex;
  }
}

std::ostream& operator<<(std::ostream& out, const StackTrace& trace) {
  std::string prev_file = "";
  for (const TraceLine& line : trace) {
    if (line.file != prev_file) {
      out << line.file << "\n";
      prev_file = line.file;
    }
    if (line.name.size()) {
      out << "    " << line.name << "\n";
    } else {
      out << "    ?? [" << std::hex << line.addr << "]\n";
    }
  }
  return out;
}

ArgType GetArgType(const std::string arg) {
  if (arg.size() > 1 && arg[0] == '-') {
    if (arg.size() > 2 && arg[1] == '-') {
      return LONG_FLAG;
    } else if (arg[1] != '-') {
      return SHORT_FLAG;
    } else {
      return POSITIONAL;
    }
  } else {
    return POSITIONAL;
  }
}

// =============================================================================
//                          String Parsing
// =============================================================================

int Parse(const std::string& str, uint8_t* value) {
  return ParseUnsigned(str, value);
}

int Parse(const std::string& str, uint16_t* value) {
  return ParseUnsigned(str, value);
}

int Parse(const std::string& str, uint32_t* value) {
  return ParseUnsigned(str, value);
}

int Parse(const std::string& str, uint64_t* value) {
  return ParseUnsigned(str, value);
}

int Parse(const std::string& str, int8_t* value) {
  return ParseSigned(str, value);
}

int Parse(const std::string& str, int16_t* value) {
  return ParseSigned(str, value);
}

int Parse(const std::string& str, int32_t* value) {
  return ParseSigned(str, value);
}

int Parse(const std::string& str, int64_t* value) {
  return ParseSigned(str, value);
}

int Parse(const std::string& str, float* value) {
  return ParseFloat(str, value);
}

int Parse(const std::string& str, double* value) {
  return ParseFloat(str, value);
}

int Parse(const std::string& str, bool* value) {
  std::string lower = ToLower(str);
  if (lower == "true" || lower == "t" || lower == "yes" || lower == "y" ||
      lower == "on" || lower == "1") {
    *value = true;
    return 0;
  } else if (lower == "false" || lower == "f" || lower == "no" ||
             lower == "n" || lower == "off" || lower == "0") {
    *value = false;
    return 0;
  } else {
    return -1;
  }
}

int Parse(const std::string& str, std::string* value) {
  *value = str;
  return 0;
}

int StringToNargs(char str) {
  if (str == '+') {
    return ONE_OR_MORE;
  } else if (str == '*') {
    return ZERO_OR_MORE;
  } else if (str == '?') {
    return ZERO_OR_ONE;
  }
  return INVALID_NARGS;
}

int StringToNargs(const char* str) {
  return StringToNargs(str[0]);
}

int StringToNargs(const std::string& str) {
  return StringToNargs(str[0]);
}

// =============================================================================
//                              Actions
// =============================================================================

ActionBase::ActionBase()
    : usage_(USAGE_POSITIONAL),
      has_nargs_{0},
      has_const_{0},
      has_default_{0},
      has_choices_{0},
      has_required_{0},
      has_help_{0},
      has_metavar_{0},
      has_destination_{0},
      nargs_(EXACTLY_ONE),
      required_{false} {}

ActionBase::~ActionBase() {}

// KWargs interface
void ActionBase::SetNargs(int nargs) {
  nargs_ = nargs;
  has_nargs_ = 1;
}
void ActionBase::SetRequired(bool required) {
  required_ = required;
  has_required_ = 1;
}
void ActionBase::SetHelp(const std::string& help) {
  help_ = help;
  has_help_ = 1;
}
void ActionBase::SetMetavar(const std::string& metavar) {
  metavar_ = metavar;
  has_metavar_ = 1;
}
void ActionBase::SetUsage(Usage usage) {
  usage_ = usage;
}

bool ActionBase::Validate() {
  return true;
}

bool ActionBase::IsRequired() const {
  if (usage_ == USAGE_POSITIONAL) {
    if (!has_nargs_) {
      return true;
    }

    if (nargs_ == ZERO_OR_MORE || nargs_ == ZERO_OR_ONE ||
        nargs_ == REMAINDER) {
      return false;
    }

    return true;
  } else {
    return (has_required_ && required_);
  }
}

int ActionBase::GetNargs(int default_value) const {
  if (has_nargs_) {
    return nargs_;
  } else {
    return default_value;
  }
}

std::string ActionBase::GetMetavar(const std::string& default_value) const {
  if (has_metavar_) {
    return metavar_;
  } else {
    return default_value;
  }
}

std::string ActionBase::GetHelp() const {
  if (this->has_help_) {
    return this->help_;
  } else {
    return "";
  }
}

Subparsers::Subparsers(const Metadata& metadata) : metadata_(metadata) {}

bool Subparsers::Validate() {
  bool ok = StoreScalar<std::string>::Validate();
  for (auto& pair : subparser_map_) {
    pair.second->Validate();
  }
  return ok;
}

std::string Subparsers::GetHelp() const {
  std::list<std::string> parts;
  if (!this->subparser_map_.empty()) {
    parts.push_back(
        fmt::format("[{}]", Join(Keys(this->subparser_map_), ", ")));
  }
  if (this->has_help_) {
    parts.push_back(this->help_);
  }
  return Join(parts, "\n");
}

void Subparsers::ConsumeArgs(const ParseContext& ctx,
                             std::list<std::string>* args,
                             ActionResult* result) {
  ARGUE_ASSERT(CONFIG_ERROR, this->nargs_ == EXACTLY_ONE)
      << fmt::format("Invalid nargs_={}", this->nargs_);
  ArgType arg_type = GetArgType(args->front());

  if (arg_type == POSITIONAL) {
    std::string local_command;
    std::string* dest = this->destination_;
    if (!dest) {
      dest = &local_command;
    }

    int parse_result = Parse(args->front(), dest);
    ARGUE_ASSERT(INPUT_ERROR, parse_result == 0)
        << fmt::format("Unable to parse command '{}'", args->front());
    args->pop_front();

    auto iter = subparser_map_.find(*dest);
    ARGUE_ASSERT(INPUT_ERROR, iter != subparser_map_.end())
        << "Invalid value '" << args->front() << "' choose from '"
        << Join(Keys(subparser_map_), "', '") << "'";

    std::shared_ptr<Parser> subparser = iter->second;
    result->code =
        static_cast<ParseResult>(subparser->ParseArgsImpl(args, ctx.out));
  } else {
    ARGUE_ASSERT(INPUT_ERROR, false) << fmt::format(
        "Expected a command name but instead got a flag {}", ctx.arg.c_str());
  }
}

Subparsers::MapType::const_iterator Subparsers::begin() const {
  return subparser_map_.begin();
}

Subparsers::MapType::const_iterator Subparsers::end() const {
  return subparser_map_.end();
}

std::shared_ptr<Parser> Subparsers::AddParser(const std::string& command,
                                              const SubparserOptions& opts) {
  auto iter = subparser_map_.find(command);
  if (iter == subparser_map_.end()) {
    Parser::Metadata meta{};
    meta.add_help = true;
    meta.add_version = false;
    meta.name = command;
    meta.prolog = opts.help;
    meta.command_prefix = metadata_.command_prefix;
    meta.subdepth = metadata_.subdepth;
    std::shared_ptr<Parser> parser{new Parser{meta}};
    std::tie(iter, std::ignore) =
        subparser_map_.emplace(std::make_pair(command, parser));
  }

  return iter->second;
}

std::string Help::GetHelp() const {
  return "print this help message";
}

bool Help::Validate() {
  return true;
}

void Help::ConsumeArgs(const ParseContext& ctx, std::list<std::string>* args,
                       ActionResult* result) {
  ctx.parser->PrintHelp(ctx.out);
  result->code = PARSE_ABORTED;
}

std::string Version::GetHelp() const {
  return "print version information and exit";
}

bool Version::Validate() {
  return true;
}

void Version::ConsumeArgs(const ParseContext& ctx, std::list<std::string>* args,
                          ActionResult* result) {
  ctx.parser->PrintVersion(ctx.out);
  result->code = PARSE_ABORTED;
}

// =============================================================================
//                              KWargs
// =============================================================================

KWargs<bool>::ActionField::ActionField()
    : std::shared_ptr<Action<bool>>(std::make_shared<StoreScalar<bool>>()) {}

KWargs<bool>::ActionField::ActionField(
    const std::shared_ptr<Action<bool>>& action)
    : std::shared_ptr<Action<bool>>(action) {}

KWargs<bool>::ActionField::ActionField(const char* named_action) {
  (*this) = named_action;
}

void KWargs<bool>::ActionField::operator=(
    const std::shared_ptr<Action<bool>>& action) {
  (*static_cast<std::shared_ptr<Action<bool>>*>(this)) = action;
}

void KWargs<bool>::ActionField::operator=(const char* named_action) {
  std::shared_ptr<Action<bool>> action;
  if (strcmp(named_action, "store") == 0) {
    action = std::make_shared<StoreScalar<bool>>();
  } else if (strcmp(named_action, "store_const") == 0) {
    action = std::make_shared<StoreConst<bool>>();
  } else if (strcmp(named_action, "store_true") == 0) {
    action = std::make_shared<StoreConst<bool>>();
    action->SetDefault(false);
    action->SetConst(true);
  } else if (strcmp(named_action, "store_false") == 0) {
    action = std::make_shared<StoreConst<bool>>();
    action->SetDefault(false);
    action->SetConst(true);
  } else {
    ARGUE_ASSERT(CONFIG_ERROR, false)
        << fmt::format("unrecognized action={}", named_action);
  }
  this->swap(action);
}

KWargs<bool>::NargsField::NargsField(int value) {
  (*this) = value;
}

KWargs<bool>::NargsField::NargsField(const char* str) {
  if (str && str[0] != '\0') {
    (*this) = str;
  }
}

void KWargs<bool>::NargsField::operator=(int value) {
  // TODO(josh): implement promotion
  container_of(this, &KWargs<bool>::nargs)->action->SetNargs(value);
}

void KWargs<bool>::NargsField::operator=(const char* str) {
  int value = StringToNargs(str);
  ARGUE_ASSERT(CONFIG_ERROR, value != INVALID_NARGS)
      << fmt::format("Invalid nargs {}", str);

  container_of(this, &KWargs<bool>::nargs)->action->SetNargs(value);
}

KWargs<bool>::ConstField::ConstField(bool value) {
  (*this) = value;
}

void KWargs<bool>::ConstField::operator=(bool value) {
  container_of(this, &KWargs<bool>::const_)->action->SetConst(value);
}

KWargs<bool>::DefaultField::DefaultField(bool value) {
  (*this) = value;
}

void KWargs<bool>::DefaultField::operator=(bool value) {
  container_of(this, &KWargs<bool>::default_)->action->SetDefault(value);
}

KWargs<bool>::DestinationField::DestinationField(bool* destination) {
  (*this) = destination;
}

void KWargs<bool>::DestinationField::operator=(bool* destination) {
  container_of(this, &KWargs<bool>::dest)->action->SetDestination(destination);
}

KWargs<bool>::RequiredField::RequiredField(bool value) {
  (*this) = value;
}

void KWargs<bool>::RequiredField::operator=(bool value) {
  container_of(this, &KWargs<bool>::required)->action->SetRequired(value);
}

KWargs<bool>::HelpField::HelpField(const std::string& value) {
  (*this) = value;
}

KWargs<bool>::HelpField::HelpField(const char* value) {
  (*this) = value;
}

void KWargs<bool>::HelpField::operator=(const std::string& value) {
  container_of(this, &KWargs<bool>::help)->action->SetHelp(value);
}

void KWargs<bool>::HelpField::operator=(const char* value) {
  container_of(this, &KWargs<bool>::help)->action->SetHelp(value);
}

KWargs<bool>::MetavarField::MetavarField(const std::string& value) {
  (*this) = value;
}

KWargs<bool>::MetavarField::MetavarField(const char* value) {
  (*this) = value;
}

void KWargs<bool>::MetavarField::operator=(const std::string& value) {
  container_of(this, &KWargs<bool>::metavar)->action->SetMetavar(value);
}

void KWargs<bool>::MetavarField::operator=(const char* value) {
  container_of(this, &KWargs<bool>::metavar)->action->SetMetavar(value);
}

KWargs<void>::ActionField::ActionField(
    const std::shared_ptr<Action<void>>& action)
    : std::shared_ptr<Action<void>>(action) {}

KWargs<void>::ActionField::ActionField(const char* named_action) {
  (*this) = named_action;
}

KWargs<void>::ActionField::ActionField(const std::string& named_action) {
  (*this) = named_action;
}

void KWargs<void>::ActionField::operator=(
    const std::shared_ptr<Action<void>>& action) {
  (*static_cast<std::shared_ptr<Action<void>>*>(this)) = action;
}

void KWargs<void>::ActionField::operator=(const char* named_action) {
  std::shared_ptr<Action<void>> action;
  if (strcmp(named_action, "help") == 0) {
    action = std::make_shared<Help>();
  } else if (strcmp(named_action, "version") == 0) {
    action = std::make_shared<Version>();
  } else {
    ARGUE_ASSERT(CONFIG_ERROR, false)
        << fmt::format("unrecognized action={}", named_action);
  }
  this->swap(action);
}

void KWargs<void>::ActionField::operator=(const std::string& named_action) {
  (*this) = named_action.c_str();
}

KWargs<void>::HelpField::HelpField(const std::string& value) {
  (*this) = value;
}

KWargs<void>::HelpField::HelpField(const char* value) {
  (*this) = value;
}

void KWargs<void>::HelpField::operator=(const std::string& value) {
  container_of(this, &KWargs<void>::help)->action->SetHelp(value);
}

void KWargs<void>::HelpField::operator=(const char* value) {
  container_of(this, &KWargs<void>::help)->action->SetHelp(value);
}

KWargs<void>::MetavarField::MetavarField(const std::string& value) {
  (*this) = value;
}

KWargs<void>::MetavarField::MetavarField(const char* value) {
  (*this) = value;
}

void KWargs<void>::MetavarField::operator=(const std::string& value) {
  container_of(this, &KWargs<void>::metavar)->action->SetMetavar(value);
}

void KWargs<void>::MetavarField::operator=(const char* value) {
  container_of(this, &KWargs<void>::metavar)->action->SetMetavar(value);
}

// =============================================================================
//                             Parser Utils
// =============================================================================

std::string GetFlagUsage(const std::string& short_flag,
                         const std::string& long_flag,
                         const std::shared_ptr<ActionBase>& action) {
  std::stringstream token;
  if (!action->IsRequired()) {
    token << "[";
  }

  std::list<std::string> parts;
  std::list<std::string> names;
  if (short_flag.size()) {
    names.emplace_back(short_flag);
  }

  std::string default_metavar = "??";
  if (!long_flag.empty()) {
    names.emplace_back(long_flag);
    default_metavar = long_flag.substr(2);
  }

  std::string name = Join(names, "/");

  int nargs = action->GetNargs(EXACTLY_ONE);
  std::string metavar = action->GetMetavar(ToUpper(default_metavar));
  if (nargs == ONE_OR_MORE) {
    parts = {name, metavar + " [..]"};
  } else if (nargs == ZERO_OR_ONE) {
    parts = {name, "[" + metavar + "]"};
  } else if (nargs == ZERO_OR_MORE) {
    parts = {name, "[" + metavar + " [..]]"};
  } else if (nargs == EXACTLY_ONE) {
    parts = {name};
  } else if (nargs > 0) {
    parts = {name, metavar, metavar, ".."};
  }

  token << Join(parts, " ");
  if (!action->IsRequired()) {
    token << "]";
  }

  return token.str();
}

std::string GetPositionalUsage(const std::string& name,
                               const std::shared_ptr<ActionBase>& action) {
  int nargs = action->GetNargs(EXACTLY_ONE);
  std::string metavar = action->GetMetavar(ToUpper(name));

  if (nargs == ONE_OR_MORE) {
    return "<" + metavar + "> [" + metavar + "..]";
  } else if (nargs == ZERO_OR_ONE) {
    return "[" + metavar + "]";
  } else if (nargs == ZERO_OR_MORE) {
    return "[" + metavar + " [" + metavar + "..]]";
  } else if (nargs == EXACTLY_ONE) {
    return "<" + metavar + ">";
  } else if (nargs > 0) {
    return fmt::format("<{0}> [{0}..]({1})", metavar, nargs);
  }

  return "";
}

const ColumnSpec kDefaultColumns = {4, 16, 60};

std::string Repeat(const std::string bit, int n) {
  std::stringstream out;
  for (int i = 0; i < n; i++) {
    out << bit;
  }
  return out.str();
}

// http://rosettacode.org/wiki/Word_wrap#C.2B.2B
std::string Wrap(const std::string text, size_t line_length) {
  std::istringstream words(text);
  std::ostringstream wrapped;
  std::string word;

  if (words >> word) {
    wrapped << word;
    size_t space_left = line_length - word.length();
    while (words >> word) {
      if (space_left < word.length() + 1) {
        wrapped << '\n' << word;
        space_left = line_length - word.length();
      } else {
        wrapped << ' ' << word;
        space_left -= word.length() + 1;
      }
    }
  }
  return wrapped.str();
}

// =============================================================================
//                                 Parser
// =============================================================================

Parser::Parser(const Metadata& meta) : meta_(meta) {
  if (meta.add_help) {
    this->AddArgument<void>("-h", "--help", {.action = "help"});
  }
  if (meta.add_version) {
    this->AddArgument<void>("-v", "--version", {.action = "version"});
  }
}

std::shared_ptr<Subparsers> Parser::AddSubparsers(
    const std::string& name, std::string* dest, const SubparserOptions& opts) {
  Subparsers::Metadata submeta{};
  submeta.command_prefix = meta_.command_prefix + " " + meta_.name;
  submeta.subdepth = meta_.subdepth + 1;
  std::shared_ptr<Subparsers> action = std::make_shared<Subparsers>(submeta);

  KWargs<std::string> spec{};
  spec.action = action;
  spec.nargs = EXACTLY_ONE;
  spec.required = true;
  spec.help = opts.help;
  spec.dest = dest;
  spec.metavar = name;

  positionals_.emplace_back(action);
  PositionalHelp help{.name = name, .action = action};

  positional_help_.emplace_back(help);
  subcommand_help_.emplace_back(action);
  return action;
}

int Parser::ParseArgs(int argc, char** argv, std::ostream* out) {
  if (argc < 0) {
    return PARSE_EXCEPTION;
  }
  if (argc > 0) {
    meta_.name = argv[0];
  }

  std::list<std::string> args;
  for (size_t i = 1; i < static_cast<size_t>(argc); ++i) {
    args.emplace_back(argv[i]);
  }

  int retcode = ParseArgs(&args, out);
  if (retcode == PARSE_EXCEPTION) {
    PrintUsage(out);
  }
  return retcode;
}

int Parser::ParseArgs(const std::initializer_list<std::string>& init_list,
                      std::ostream* out) {
  std::list<std::string> args = init_list;
  return ParseArgs(&args, out);
}

int Parser::ParseArgs(std::list<std::string>* args, std::ostream* out) {
  try {
    return ParseArgsImpl(args, out);
  } catch (const Exception& ex) {
    (*out) << Exception::ToString(ex.typeno) << ": ";
    (*out) << ex.message << "\n";
    (*out) << ex.stack_trace;
    return PARSE_EXCEPTION;
  }
}

void Parser::Validate() {
  for (auto& action : positionals_) {
    action->Validate();
  }
  for (auto& pair : short_flags_) {
    pair.second.action->Validate();
  }
  for (auto& pair : long_flags_) {
    pair.second.action->Validate();
  }
}

int Parser::ParseArgsImpl(std::list<std::string>* args, std::ostream* out) {
  this->Validate();
  ParseContext ctx{.parser = this, .out = out};

  auto positionals = positionals_;
  auto short_flags = short_flags_;
  auto long_flags = long_flags_;

  while (args->size() > 0) {
    ArgType arg_type = GetArgType(args->front());
    ActionResult out{
        .keep_active = false,
        .code = PARSE_FINISHED,
    };

    switch (arg_type) {
      case SHORT_FLAG: {
        ctx.arg = args->front();
        args->pop_front();
        for (size_t idx = 1; idx < ctx.arg.size(); ++idx) {
          std::string query_flag = std::string("-") + ctx.arg[idx];
          auto flag_iter = short_flags.find(query_flag);
          ARGUE_ASSERT(INPUT_ERROR, (flag_iter != short_flags.end()))
              << "Unrecognized short flag: " << query_flag;
          FlagStore store = flag_iter->second;
          ARGUE_ASSERT(BUG, static_cast<bool>(store.action))
              << "Flag " << query_flag
              << " was found in index with empty action pointer";
          store.action->ConsumeArgs(ctx, args, &out);

          if (!out.keep_active) {
            short_flags.erase(store.short_flag);
            long_flags.erase(store.long_flag);
          }
        }
        break;
      }

      case LONG_FLAG: {
        ctx.arg = args->front();
        args->pop_front();
        auto flag_iter = long_flags.find(ctx.arg);
        ARGUE_ASSERT(INPUT_ERROR, (flag_iter != long_flags.end()))
            << "Unrecognized long flag: " << ctx.arg;
        FlagStore store = flag_iter->second;
        ARGUE_ASSERT(BUG, static_cast<bool>(store.action))
            << "Flag " << ctx.arg
            << " was found in index with empty action pointer";
        store.action->ConsumeArgs(ctx, args, &out);
        if (!out.keep_active) {
          short_flags.erase(store.short_flag);
          long_flags.erase(store.long_flag);
        }
        break;
      }

      case POSITIONAL: {
        ctx.arg = "";
        ARGUE_ASSERT(CONFIG_ERROR, positionals.size() > 0)
            << "Additional positional arguments with no available actions "
               "remaining: '"
            << args->front() << "'";
        std::shared_ptr<ActionBase> action = positionals.front();
        positionals.pop_front();
        ARGUE_ASSERT(BUG, static_cast<bool>(action))
            << "positional with empty action pointer";
        action->ConsumeArgs(ctx, args, &out);
        break;
      }
    }

    if (out.code != PARSE_FINISHED) {
      return out.code;
    }
  }

  for (const std::shared_ptr<ActionBase>& action : positionals) {
    ARGUE_ASSERT(INPUT_ERROR, !action->IsRequired())
        << "Missing required positional";
  }

  for (const auto& pair : short_flags_) {
    const FlagStore& store = pair.second;
    ARGUE_ASSERT(INPUT_ERROR, !store.action->IsRequired())
        << "Missing required flag (" << store.short_flag << ","
        << store.long_flag << ")";
  }

  for (const auto& pair : long_flags_) {
    const FlagStore& store = pair.second;
    ARGUE_ASSERT(INPUT_ERROR, !store.action->IsRequired())
        << "Missing required flag (" << store.short_flag << ","
        << store.long_flag << ")";
  }

  return PARSE_FINISHED;
}

void Parser::PrintUsage(std::ostream* out, size_t width) {
  std::stringstream line;

  std::list<std::string> parts;
  if (!meta_.command_prefix.empty()) {
    parts.push_back(meta_.command_prefix);
  }
  parts.push_back(meta_.name);
  for (const FlagHelp& help : flag_help_) {
    parts.push_back(GetFlagUsage(help.short_flag, help.long_flag, help.action));
  }

  for (const PositionalHelp& help : positional_help_) {
    parts.push_back(GetPositionalUsage(help.name, help.action));
  }

  (*out) << Join(parts, " ") << "\n";
}

static void PrintColumns(std::ostream* out, const ColumnSpec& columns,
                         const std::string& name,
                         const std::string& description) {
  size_t width = 80;
  size_t padding = (width - ContainerSum(columns)) / (columns.size() - 1);

  if (name.size() > padding + columns[0] + columns[1]) {
    (*out) << "\n";
  }
  (*out) << name;
  (*out) << Repeat(" ", 2 * padding + columns[0] + columns[1] - name.size());
  if (name.size() > padding + columns[0] + columns[1]) {
    (*out) << "\n";
    (*out) << Repeat(" ", columns[0] + columns[1] + 2 * padding);
  }

  std::stringstream ss(Wrap(description, columns[2]));
  std::string line;

  if (std::getline(ss, line, '\n')) {
    (*out) << line << "\n";
  } else {
    (*out) << "\n";
  }

  while (std::getline(ss, line, '\n')) {
    (*out) << Repeat(" ", columns[0] + columns[1] + 2 * padding) << line
           << "\n";
  }
}

void Parser::PrintHelp(std::ostream* out, const HelpOptions& opts) {
  const ColumnSpec columns = opts.columns;
  size_t width = 80;
  size_t padding = (width - ContainerSum(columns)) / (columns.size() - 1);

  // TODO(josh): detect multiline and break it up
  if (meta_.subdepth < 1) {
    (*out) << Repeat("=", meta_.name.size()) << "\n"
           << meta_.name << "\n"
           << Repeat("=", meta_.name.size()) << "\n";

    if (meta_.version.size() > 0) {
      (*out) << "version: " << Join(meta_.version, ".") << "\n";
    }
    if (meta_.author.size() > 0) {
      (*out) << "author : " << meta_.author << "\n";
    }
    if (meta_.copyright.size() > 0) {
      (*out) << "copyright: " << meta_.copyright << "\n";
    }
    (*out) << "\n";
  }

  PrintUsage(out, width);

  if (meta_.prolog.size() > 0) {
    (*out) << "\n" << meta_.prolog << "\n";
  }

  if (flag_help_.size() > 0) {
    if (opts.depth < 1) {
      (*out) << "\n";
      (*out) << "Flags:\n";
      (*out) << Repeat("-", 6) << "\n";
    } else {
      (*out) << Repeat("-", 4) << "\n";
    }
    for (const FlagHelp& help : flag_help_) {
      // If we're going to overflow our column, then also push a newline
      // between us and the previous one for some additional padding
      if (help.long_flag.size() > columns[1]) {
        (*out) << "\n";
      }
      (*out) << help.short_flag;
      (*out) << Repeat(" ", padding + columns[0] - help.short_flag.size());
      (*out) << help.long_flag;
      (*out) << Repeat(" ", padding + columns[1] - help.long_flag.size());

      // If we overflowed the column, then add a new line so that we can
      // start the help text at the right column
      if (help.long_flag.size() > columns[1]) {
        (*out) << "\n";
        (*out) << Repeat(" ", columns[0] + columns[1] + 2 * padding);
      }

      std::stringstream ss(Wrap(help.action->GetHelp(), columns[2]));
      std::string line;

      if (std::getline(ss, line, '\n')) {
        (*out) << line << "\n";
      } else {
        (*out) << "\n";
      }

      while (std::getline(ss, line, '\n')) {
        (*out) << Repeat(" ", columns[0] + columns[1] + 2 * padding) << line
               << "\n";
      }
    }
  }

  if (positional_help_.size() > 0) {
    if (opts.depth < 1) {
      (*out) << "\n";
      (*out) << "Positionals:\n";
      (*out) << Repeat("-", 12) << "\n";
    } else {
      (*out) << Repeat("-", 4) << "\n";
    }
    for (const PositionalHelp& help : positional_help_) {
      PrintColumns(out, columns, help.name, help.action->GetHelp());
    }
  }

  if (opts.depth < 1 && subcommand_help_.size() > 0) {
    (*out) << "\n";
    (*out) << "Subcommands:\n" << Repeat("-", 12) << "\n";
    for (const auto& sub : subcommand_help_) {
      for (auto& pair : *sub) {
        PrintColumns(out, columns, pair.first, pair.second->GetProlog());
      }
    }
  }

  if (meta_.epilog.size() > 0) {
    (*out) << meta_.epilog;
  }
}

void Parser::PrintVersion(std::ostream* out, const ColumnSpec& columns) {
  // TODO(josh): detect multiline and break it up
  (*out) << meta_.name << " ";
  if (meta_.version.size() > 0) {
    (*out) << "  version " << Join(meta_.version, ".") << "\n";
  }
}

const std::string& Parser::GetProlog() {
  return meta_.prolog;
}

}  // namespace argue
