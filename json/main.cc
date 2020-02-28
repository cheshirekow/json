// Copyright 2018 Josh Bialkowski <josh.bialkowski@gmail.com>
#include <fstream>

#include "argue/argue.h"
#include "json/json.h"

struct ProgramOpts {
  std::string command;
  std::string infile = "-";

  struct {
    bool omit_template;
  } markup;
};

int LexFile(const ProgramOpts& opts, const std::string& content) {
  json::Error error{};
  json::Scanner scanner;
  scanner.Init(&error);
  scanner.Begin(content);
  json::Token token;

  uint32_t idx = 0;
  while (scanner.Pump(&token, &error) == 0) {
    printf("%3d: [%14s](%d:%d) '%.*s'\n", idx++,
           json::Token::ToString(token.typeno), token.location.lineno,
           token.location.colno, static_cast<int>(token.spelling.size()),
           token.spelling.begin());
  }
  if (error.code == json::Error::LEX_INPUT_FINISHED) {
    return 0;
  } else {
    std::cerr << error.msg << "\n";
    return error.code;
  }
}

int ParseFile(const ProgramOpts& opts, const std::string& content) {
  json::Error error{};
  json::LexerParser parser;
  parser.Init(&error);
  parser.Begin(content);
  json::Event event;
  uint32_t idx = 0;
  while (parser.GetNextEvent(&event, &error) == 0) {
    printf("%3d: [%13s] '%.*s'\n", idx++, json::Event::ToString(event.typeno),
           static_cast<int>(event.token.spelling.size()),
           event.token.spelling.begin());
  }
  if (error.code == json::Error::LEX_INPUT_FINISHED) {
    exit(0);
  } else {
    std::cerr << error.msg << "\n";
    exit(error.code);
  }
}

int VerifyFile(const ProgramOpts& opts, const std::string& content) {
  json::Error error{};
  int result = json::Verify(content, &error);
  if (result != 0) {
    fprintf(stderr, "%s", error.msg);
  }
  return result;
}

const char* kMarkupHead =
    ""
    "<html>\n"
    "<head>\n"
    "<style type=\"text/css\">\n"
    "body {\n"
    "  background-color: #1e1e1e;\n"
    "  color: #d4d4d4;\n"
    "}\n"
    "div.content {\n"
    "  white-space: pre;\n"
    "  font-family: 'Courier New', Courier, monospace;\n"
    "}\n"
    "span.COMMENT {\n"
    "  color: darkgrey;\n"
    "}\n"
    "span.BOOLEAN_LITERAL, span.NULL_LITERAL {\n"
    "  color: violet;\n"
    "  font-weight: bold;\n"
    "}\n"
    "span.NUMERIC_LITERAL {\n"
    "  color: lightblue;\n"
    "  font-weight: bold;\n"
    "}\n"
    "span:not(.OBJECT_KEY) > span.STRING_LITERAL {\n"
    "  color: lightgreen;\n"
    "}\n"
    "</style>\n"
    "</head>\n"
    "<body>\n"
    "<div class=\"content\">\n";

const char* kMarkupTail =
    ""
    "</div>\n"
    "</body>\n"
    "</html>\n";

int MarkupFile(const ProgramOpts& opts, const std::string& content) {
  json::Error error{};
  json::Scanner scanner;
  json::Parser parser;
  json::Token token;
  json::Event event;

  scanner.Init(&error);
  scanner.Begin(content);

  if (!opts.markup.omit_template) {
    printf("%s", kMarkupHead);
  }
  while (scanner.Pump(&token, &error) == 0) {
    int err = parser.HandleToken(token, &event, &error);
    if (err < 0) {
      break;
    }

    if (err > 0) {
      switch (event.typeno) {
        case json::Event::OBJECT_BEGIN:
        case json::Event::OBJECT_KEY:
        case json::Event::LIST_BEGIN:
        case json::Event::VALUE_LITERAL:
          printf("<span class=\"%s\">", json::Event::ToString(event.typeno));
        default:
          break;
      }

      printf("<span class=\"%s\">%.*s</span>",
             json::Token::ToString(token.typeno),
             static_cast<int>(token.spelling.size()), token.spelling.begin());

      switch (event.typeno) {
        case json::Event::OBJECT_END:
        case json::Event::OBJECT_KEY:
        case json::Event::LIST_END:
        case json::Event::VALUE_LITERAL:
          printf("</span>");
        default:
          break;
      }
    } else {
      printf("<span class=\"%s\">%.*s</span>",
             json::Token::ToString(token.typeno),
             static_cast<int>(token.spelling.size()), token.spelling.begin());
    }
  }
  printf("\n");
  if (!opts.markup.omit_template) {
    printf("%s", kMarkupTail);
  }
  if (error.code == json::Error::LEX_INPUT_FINISHED) {
    return 0;
  } else {
    std::cerr << error.msg << "\n";
    return error.code;
  }
}

const char* kProlog =
    ""
    "Demonstrates the usage of the json library to lex and parse JSON data";

int main(int argc, char** argv) {
  argue::Parser parser({.add_help = true,
                        .add_version = true,
                        .name = "json",
                        .version = JSON_VERSION,
                        .author = "Josh Bialkowski <josh.bialkowski@gmail.com>",
                        .copyright = "(C) 2018",
                        .prolog = kProlog});
  ProgramOpts opts;
  auto subparsers = parser.AddSubparsers(
      "command", &opts.command,
      {.help = "Each subcommand has it's own options and arguments, see "
               "individual subcommand help."});
  auto lex_parser = subparsers->AddParser(
      "lex", {.help = "Lex the file and dump token information"});
  auto parse_parser = subparsers->AddParser(
      "parse", {.help = "Parse the file and dump actionable parse events"});
  auto verify_parser = subparsers->AddParser(
      "verify", {.help = "Parse the file and exit with 0 if it's valid json"});
  auto markup_parser = subparsers->AddParser(
      "markup", {.help = "Parse and dump the contents with HTML markup"});

  for (auto& subparser :
       {lex_parser, parse_parser, markup_parser, verify_parser}) {
    argue::KWargs<std::string> kwargs{.action = "store",
                                      .nargs = "?",
                                      .const_ = {},
                                      .default_ = "-",
                                      .choices = {},
                                      .dest = {},
                                      .required = false,
                                      .help = "Path to input, '-' for stdin",
                                      .metavar = "infile"};

    subparser->AddArgument("infile", &opts.infile,
                           {.action = "store",
                            .nargs = "?",
                            .const_ = {},
                            .default_ = "-",
                            .choices = {},
                            .dest = {},
                            .required = false,
                            .help = "Path to input, '-' for stdin",
                            .metavar = "infile"});
  }

  argue::KWargs<bool> argopts{};
  argopts.action = "store_true";
  argopts.help = "output just the content";
  markup_parser->AddArgument("-o", "--omit-template",
                             &opts.markup.omit_template, argopts);

  int parse_result = parser.ParseArgs(argc, argv);
  switch (parse_result) {
    case argue::PARSE_ABORTED:
      return 0;
    case argue::PARSE_EXCEPTION:
      return 1;
    case argue::PARSE_FINISHED:
      break;
  }

  std::istream* infile;
  std::ifstream open_infile;
  if (opts.infile == "-") {
    infile = &std::cin;
  } else {
    open_infile.open(opts.infile);
    infile = &open_infile;
  }

  std::string content;
  content.reserve(1024 * 1024);
  content.assign((std::istreambuf_iterator<char>(*infile)),
                 std::istreambuf_iterator<char>());

  if (opts.command == "lex") {
    exit(LexFile(opts, content));
  } else if (opts.command == "parse") {
    exit(ParseFile(opts, content));
  } else if (opts.command == "verify") {
    exit(VerifyFile(opts, content));
  } else if (opts.command == "markup") {
    exit(MarkupFile(opts, content));
  } else {
    printf("Unknown command\n");
  }
  return 0;
}
