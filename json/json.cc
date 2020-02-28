// Copyright 2018 Josh Bialkowski <josh.bialkowski@gmail.com>
#include "json/json.h"

#include <algorithm>
#include <array>
#include <cstdarg>
#include <memory>
#include <string>
#include <vector>

#include <re2/re2.h>

#include "util/fixed_string_stream.h"

namespace json {

// -----------------------------------------------------------------------------
//    Token
// -----------------------------------------------------------------------------

const char* kTokenTypeToString[] = {
    "STRING_LITERAL",   //
    "NUMERIC_LITERAL",  //
    "BOOLEAN_LITERAL",  //
    "NULL_LITERAL",     //
    "WHITESPACE",       //
    "PUNCTUATION",      //
    "COMMENT",          //
};

const char* Token::ToString(TypeNo no) {
  return SafeGet(kTokenTypeToString, no);
}

// -----------------------------------------------------------------------------
//    Error
// -----------------------------------------------------------------------------

const char* kErrorCodeToString[] = {
    "NOERROR",                 //
    "INTERNAL_ERROR",          //
    "LEX_INPUT_FINISHED",      //
    "LEX_INVALID_TOKEN",       //
    "PARSE_UNEXPECTED_TOKEN",  //
    "PARSE_OOM",               //
    "PARSE_BAD_STATE",         //
};

const char* Error::ToString(Code no) {
  return SafeGet(kErrorCodeToString, no);
}

// -----------------------------------------------------------------------------
//    Event
// -----------------------------------------------------------------------------

const char* kEventTypeNoToString[] = {
    "OBJECT_BEGIN",   //
    "OBJECT_KEY",     //
    "OBJECT_END",     //
    "LIST_BEGIN",     //
    "LIST_END",       //
    "VALUE_LITERAL",  //
    "INVALID",
};

const char* Event::ToString(TypeNo no) {
  return SafeGet(kEventTypeNoToString, no);
}

// -----------------------------------------------------------------------------
//   Scanner
// -----------------------------------------------------------------------------

//! Scanner specification item is a string pattern and the token type that it
//! maps to
struct Spec {
  std::string pattern;
  Token::TypeNo typeno;
};

//! Mapping of string patern to token type
Spec kScanList[] = {
    // https://stackoverflow.com/a/37379449/141023
    {"\"[^\"\\\\]*(?:\\\\.[^\"\\\\]*)*\"", Token::STRING_LITERAL},
    {"(-?\\d+)(\\.\\d+)?([eE][+-]?\\d+)?", Token::NUMERIC_LITERAL},
    {"true", Token::BOOLEAN_LITERAL},
    {"false", Token::BOOLEAN_LITERAL},
    {"null", Token::NULL_LITERAL},
    {"\\s+", Token::WHITESPACE},
    // TODO(josh): only allow these if configured
    {"//[^\n]+\n", Token::COMMENT},
    {"#[^\n]+\n", Token::COMMENT},
};

int Scanner::Init(Error* error) {
  if (init_state_) {
    return init_state_;
  }

  matches_.reserve(sizeof(kScanList) / sizeof(Spec));
  for (size_t idx = 0; idx < sizeof(kScanList) / sizeof(Spec); ++idx) {
    int err = scanset_.Add(kScanList[idx].pattern, NULL);
    if (err != static_cast<int>(idx)) {
      FmtError(error, Error::INTERNAL_ERROR)
          << "Failed to add all scanlist items to `RE2::Set`";
      init_state_ = -1;
      return init_state_;
    }
  }
  if (!scanset_.Compile()) {
    FmtError(error, Error::INTERNAL_ERROR) << "Failed to compile `RE2::Set`";
    init_state_ = -1;
    return init_state_;
  }

  init_state_ = 1;
  return 0;
}

int Scanner::Begin(const re2::StringPiece& piece) {
  piece_ = piece;
  numeric_storage_ = 0;
  string_storage_ = 0;
  loc_ = {0, 0, 0};
  return 0;
}

void AdvanceLocation(const re2::StringPiece& str, SourceLocation* loc) {
  for (size_t idx = 0; idx < str.size(); ++idx) {
    if (str[idx] == '\n') {
      loc->lineno++;
      loc->colno = 0;
    } else {
      loc->colno++;
    }
    loc->offset++;
  }
}

int Scanner::Pump(Token* tok, Error* error) {
  if (piece_.size() < 1) {
    FmtError(error, Error::LEX_INPUT_FINISHED, loc_)
        << "The input stream is empty. Either parsing is finished or the data "
           "is truncated.";
    return -1;
  }

  switch (piece_[0]) {
    case ':':
    case ',':
    case '{':
    case '}':
    case '[':
    case ']':
      tok->typeno = Token::PUNCTUATION;
      tok->spelling = piece_.substr(0, 1);
      tok->location = loc_;
      piece_ = piece_.substr(1);
      AdvanceLocation(tok->spelling, &loc_);
      return 0;
    default:
      break;
  }

  matches_.resize(0);
  if (!scanset_.Match(piece_, &matches_)) {
    FmtError(error, Error::LEX_INVALID_TOKEN, loc_)
        << "An invalid input token was encountered. Source is not valid json. "
        << "At " << loc_.lineno << ":" << loc_.colno;

    return -1;
  }

  auto begin = piece_.begin();

  int match_idx = *std::min_element(matches_.begin(), matches_.end());
  if (!RE2::Consume(&piece_, kScanList[match_idx].pattern)) {
    FmtError(error, Error::INTERNAL_ERROR, loc_)
        << "A valid token was matched but RE2 was unable to consume it";
    return -1;
  }
  auto end = piece_.begin();
  tok->typeno = kScanList[match_idx].typeno;
  tok->spelling = re2::StringPiece(begin, end - begin);
  tok->location = loc_;
  AdvanceLocation(tok->spelling, &loc_);

  switch (match_idx) {
    case Token::NUMERIC_LITERAL:
      numeric_storage_ += sizeof(uint32_t);
      break;
    case Token::STRING_LITERAL:
      string_storage_ += tok->spelling.size() + 1;
      break;
    default:
      break;
  }

  return 0;
}

re2::StringPiece Scanner::GetPiece() {
  return piece_;
}

// -----------------------------------------------------------------------------
//   High Level Lex Functions
// -----------------------------------------------------------------------------

int Lex(const re2::StringPiece& source, Token* buf, size_t n, Error* error) {
  Scanner scanner{};

  if (scanner.Init(error)) {
    return -1;
  }

  if (scanner.Begin(source)) {
    return -2;
  }

  Error local_error;
  if (!error) {
    error = &local_error;
  }

  size_t ntokens = 0;
  for (; ntokens < n; ++ntokens) {
    if (scanner.Pump(&buf[ntokens], error) < 0) {
      if (error->code == Error::LEX_INPUT_FINISHED) {
        return ntokens;
      } else {
        return -3;
      }
    }
  }

  Token local_token;
  while (scanner.Pump(&local_token, error) >= 0) {
    ntokens++;
  }

  if (error->code == Error::LEX_INPUT_FINISHED) {
    return ntokens;
  } else {
    return -3;
  }
}

int VerifyLex(const re2::StringPiece& source, Error* error) {
  int result = Lex(source, nullptr, 0, error);
  if (result < 0) {
    return result;
  } else {
    return 0;
  }
}

// -----------------------------------------------------------------------------
//    Parser
// -----------------------------------------------------------------------------

Parser::Parser() : state_(PARSING_VALUE) {}

void Parser::Reset() {
  state_ = PARSING_VALUE;
  group_stack_.clear();
}

int Parser::HandleToken(const Token& tok, Event* event, Error* error) {
  event->token = tok;
  if (tok.typeno == Token::WHITESPACE || tok.typeno == Token::COMMENT) {
    return 0;
  }

  switch (state_) {
    case PARSING_VALUE: {
      if (tok.typeno == Token::PUNCTUATION) {
        if (tok.spelling != "{" && tok.spelling != "[") {
          FmtError(error, Error::PARSE_UNEXPECTED_TOKEN, tok.location)
              << "Expected '{' or '[' but got " << tok.spelling;
          return -1;
        }

        if (tok.spelling == "{") {
          group_stack_.push_back(Event::OBJECT_BEGIN);
          state_ = PARSING_KEY;
          event->typeno = Event::OBJECT_BEGIN;
        } else {
          group_stack_.push_back(Event::LIST_BEGIN);
          state_ = PARSING_VALUE;
          event->typeno = Event::LIST_BEGIN;
        }
        return 1;
      } else {
        event->typeno = Event::VALUE_LITERAL;
        state_ = PARSING_CLOSURE;
        return 1;
      }
      break;
    }

    case PARSING_KEY: {
      if (tok.typeno != Token::STRING_LITERAL) {
        FmtError(error, Error::PARSE_UNEXPECTED_TOKEN, tok.location)
            << "Expected a string literal (key) but got " << tok.spelling;
        return -1;
      }

      if (!group_stack_.size()) {
        FmtError(error, Error::INTERNAL_ERROR, tok.location)
            << "group_stack_ is empty" << tok.spelling;
        return -1;
      }

      if (group_stack_.back() != Event::OBJECT_BEGIN) {
        FmtError(error, Error::INTERNAL_ERROR, tok.location)
            << "group_stack_ is not an object" << tok.spelling;
        return -1;
      }

      event->typeno = Event::OBJECT_KEY;
      state_ = PARSING_COLON;
      return 1;
    }

    case PARSING_COLON: {
      if (tok.typeno != Token::PUNCTUATION || tok.spelling != ":") {
        FmtError(error, Error::PARSE_UNEXPECTED_TOKEN, tok.location)
            << "Expected a colon (':') but got " << tok.spelling;
        return -1;
      }
      state_ = PARSING_VALUE;
      return 0;
    }

    case PARSING_CLOSURE: {
      if (tok.typeno != Token::PUNCTUATION) {
        FmtError(error, Error::PARSE_UNEXPECTED_TOKEN, tok.location)
            << "Expected ']', '}', or ',' but got " << tok.spelling;
        return -1;
      }

      if (!group_stack_.size()) {
        FmtError(error, Error::INTERNAL_ERROR, tok.location)
            << "group_stack_ is empty" << tok.spelling;
        return -1;
      }

      if (tok.spelling == ",") {
        if (group_stack_.back() == Event::LIST_BEGIN) {
          state_ = PARSING_VALUE;
          return 0;
        } else if (group_stack_.back() == Event::OBJECT_BEGIN) {
          state_ = PARSING_KEY;
          return 0;
        } else {
          FmtError(error, Error::INTERNAL_ERROR, tok.location)
              << "Top of group stack is not a list or object" << tok.spelling;
          return -1;
        }
      }

      if (group_stack_.back() == Event::LIST_BEGIN) {
        if (tok.spelling != "]") {
          FmtError(error, Error::PARSE_UNEXPECTED_TOKEN, tok.location)
              << "Expected ']' but got " << tok.spelling;
          return -1;
        }

        group_stack_.pop_back();
        state_ = PARSING_CLOSURE;
        event->typeno = Event::LIST_END;
        return 1;
      }

      if (group_stack_.back() == Event::OBJECT_BEGIN) {
        if (tok.spelling != "}") {
          FmtError(error, Error::PARSE_UNEXPECTED_TOKEN, tok.location)
              << "Expected '}' but got " << tok.spelling;
          return -1;
        }

        group_stack_.pop_back();
        event->typeno = Event::OBJECT_END;
        return 1;
      }

      FmtError(error, Error::PARSE_UNEXPECTED_TOKEN, tok.location)
          << "Expected ']', '}', or ',' but got " << tok.spelling;
      return -1;
    }

    case PARSING_ERROR: {
      FmtError(error, Error::PARSE_BAD_STATE, tok.location)
          << "Parser is in an error state";
      return -1;
    }

    default: {
      FmtError(error, Error::INTERNAL_ERROR, tok.location)
          << "Unknown parser state: " << state_ << tok.spelling;
      return -1;
    }
  }

  return 0;
}

// -----------------------------------------------------------------------------
//    LexerParser
// -----------------------------------------------------------------------------

int LexerParser::Init(Error* error) {
  return scanner_.Init(error);
}

int LexerParser::Begin(const re2::StringPiece& string) {
  parser_.Reset();
  return scanner_.Begin(string);
}

int LexerParser::GetNextEvent(Event* event, Error* error) {
  while (true) {
    int result = scanner_.Pump(&token_, error);
    if (result < 0) {
      return result;
    }

    result = parser_.HandleToken(token_, event, error);
    if (result < 0) {
      return result;
    }
    if (result > 0) {
      return 0;
    }
  }
}

// -----------------------------------------------------------------------------
//    High Level Parse Functinos
// -----------------------------------------------------------------------------

int Parse(const re2::StringPiece& source, Event* buf, size_t n, Error* error) {
  LexerParser parser{};

  if (parser.Init(error)) {
    return -1;
  }
  if (parser.Begin(source)) {
    return -2;
  }

  Error local_error;
  if (!error) {
    error = &local_error;
  }

  size_t nevents = 0;
  for (; nevents < n; ++nevents) {
    int result = parser.GetNextEvent(&buf[nevents], error);
    if (result < 0) {
      if (error->code == Error::LEX_INPUT_FINISHED) {
        return nevents;
      } else {
        return -3;
      }
    }
  }

  Event local_event;
  for (; true; ++nevents) {
    int result = parser.GetNextEvent(&local_event, error);
    if (result < 0) {
      if (error->code == Error::LEX_INPUT_FINISHED) {
        return nevents;
      } else {
        return -5;
      }
    }
  }
}

int Verify(const re2::StringPiece& source, Error* error) {
  if (Parse(source, nullptr, 0, error) >= 0) {
    return 0;
  } else {
    return -1;
  }
}

const SerializeOpts kDefaultOpts{
    .indent = 2,
    .separators = {": ", ","},
};

const json::SerializeOpts kCompactOpts = {  //
    .indent = 0,
    .separators = {":", ","}};

util::FixedBufStream<char> FmtError(Error* error, Error::Code code,
                                    SourceLocation loc) {
  if (error) {
    error->code = code;
    error->loc = loc;
    return util::FixedBufStream<char>(&error->msg);
  } else {
    return util::FixedBufStream<char>(static_cast<char*>(nullptr),
                                      static_cast<size_t>(0));
  }
}

}  // namespace json
