// Copyright 2018 Josh Bialkowski <josh.bialkowski@gmail.com>
#include "tangent/json/json.h"

#include <algorithm>
#include <array>
#include <cstdarg>
#include <memory>
#include <string>
#include <vector>

#include <re2/re2.h>

#include "tangent/util/fixed_string_stream.h"

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

const char* Token::to_string(TypeNo no) {
  return safe_get(kTokenTypeToString, no);
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

const char* Error::to_string(Code no) {
  return safe_get(kErrorCodeToString, no);
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

const char* Event::to_string(TypeNo no) {
  return safe_get(kEventTypeNoToString, no);
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

int Scanner::init(Error* error) {
  if (init_state_) {
    return init_state_;
  }

  matches_.reserve(sizeof(kScanList) / sizeof(Spec));
  for (size_t idx = 0; idx < sizeof(kScanList) / sizeof(Spec); ++idx) {
    int err = scanset_.Add(kScanList[idx].pattern, NULL);
    if (err != static_cast<int>(idx)) {
      fmt_error(error, Error::INTERNAL_ERROR)
          << "Failed to add all scanlist items to `RE2::Set`";
      init_state_ = -1;
      return init_state_;
    }
  }
  if (!scanset_.Compile()) {
    fmt_error(error, Error::INTERNAL_ERROR) << "Failed to compile `RE2::Set`";
    init_state_ = -1;
    return init_state_;
  }

  init_state_ = 1;
  return 0;
}

int Scanner::begin(const re2::StringPiece& piece) {
  piece_ = piece;
  numeric_storage_ = 0;
  string_storage_ = 0;
  loc_ = {0, 0, 0};
  return 0;
}

void advance_location(const re2::StringPiece& str, SourceLocation* loc) {
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

int Scanner::pump_impl(Token* tok, Error* error, bool peek) {
  if (piece_.size() < 1) {
    fmt_error(error, Error::LEX_INPUT_FINISHED, loc_)
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
      if (peek) {
        return 0;
      }
      piece_ = piece_.substr(1);
      advance_location(tok->spelling, &loc_);
      return 0;
    default:
      break;
  }

  matches_.resize(0);
  if (!scanset_.Match(piece_, &matches_)) {
    fmt_error(error, Error::LEX_INVALID_TOKEN, loc_)
        << "An invalid input token was encountered. Source is not valid json. "
        << "At " << loc_.lineno << ":" << loc_.colno;

    return -1;
  }

  auto begin = piece_.begin();
  int match_idx = *std::min_element(matches_.begin(), matches_.end());
  re2::StringPiece mutable_piece = piece_;
  if (!RE2::Consume(&mutable_piece, kScanList[match_idx].pattern)) {
    fmt_error(error, Error::INTERNAL_ERROR, loc_)
        << "A valid token was matched but RE2 was unable to consume it";
    return -1;
  }
  auto end = mutable_piece.begin();
  tok->typeno = kScanList[match_idx].typeno;
  tok->spelling = re2::StringPiece(begin, end - begin);
  tok->location = loc_;

  if (peek) {
    return 0;
  }

  piece_ = mutable_piece;
  advance_location(tok->spelling, &loc_);

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

re2::StringPiece Scanner::get_piece() {
  return piece_;
}

// -----------------------------------------------------------------------------
//   High Level Lex Functions
// -----------------------------------------------------------------------------

int lex(const re2::StringPiece& source, Token* buf, size_t n, Error* error) {
  Scanner scanner{};

  if (scanner.init(error)) {
    return -1;
  }

  if (scanner.begin(source)) {
    return -2;
  }

  Error local_error;
  if (!error) {
    error = &local_error;
  }

  size_t ntokens = 0;
  for (; ntokens < n; ++ntokens) {
    if (scanner.pump(&buf[ntokens], error) < 0) {
      if (error->code == Error::LEX_INPUT_FINISHED) {
        return ntokens;
      } else {
        return -3;
      }
    }
  }

  Token local_token;
  while (scanner.pump(&local_token, error) >= 0) {
    ntokens++;
  }

  if (error->code == Error::LEX_INPUT_FINISHED) {
    return ntokens;
  } else {
    return -3;
  }
}

int verify_lex(const re2::StringPiece& source, Error* error) {
  int result = lex(source, nullptr, 0, error);
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

void Parser::reset() {
  state_ = PARSING_VALUE;
  group_stack_.clear();
}

int Parser::handle_token(const Token& tok, Event* event, Error* error,
                         bool dry_run) {
  event->token = tok;
  if (tok.typeno == Token::WHITESPACE || tok.typeno == Token::COMMENT) {
    return 0;
  }

  switch (state_) {
    case PARSING_LIST_OPEN: {
      if (tok.typeno == Token::PUNCTUATION && !group_stack_.empty() &&
          group_stack_.back() == Event::LIST_BEGIN && tok.spelling == "]") {
        event->typeno = Event::LIST_END;
        if (dry_run) {
          return 1;
        }
        group_stack_.pop_back();
        state_ = PARSING_CLOSURE;
        return 1;
      } else {
        // fallthrough
      }
    }

    case PARSING_VALUE: {
      if (tok.typeno == Token::PUNCTUATION) {
        if (tok.spelling != "{" && tok.spelling != "[") {
          fmt_error(error, Error::PARSE_UNEXPECTED_TOKEN, tok.location)
              << "Expected '{' or '[' but got " << tok.spelling;
          return -1;
        }

        if (tok.spelling == "{") {
          event->typeno = Event::OBJECT_BEGIN;
          if (dry_run) {
            return 1;
          }
          group_stack_.push_back(Event::OBJECT_BEGIN);
          state_ = PARSING_OBJECT_OPEN;
        } else {
          event->typeno = Event::LIST_BEGIN;
          if (dry_run) {
            return 1;
          }
          group_stack_.push_back(Event::LIST_BEGIN);
          state_ = PARSING_LIST_OPEN;
        }
        return 1;
      } else {
        event->typeno = Event::VALUE_LITERAL;
        if (dry_run) {
          return 1;
        }
        state_ = PARSING_CLOSURE;
        return 1;
      }
      break;
    }

    case PARSING_OBJECT_OPEN: {
      if (tok.typeno == Token::PUNCTUATION && !group_stack_.empty() &&
          group_stack_.back() == Event::OBJECT_BEGIN && tok.spelling == "}") {
        event->typeno = Event::OBJECT_END;
        if (dry_run) {
          return 1;
        }
        group_stack_.pop_back();
        state_ = PARSING_CLOSURE;
        return 1;
      } else {
        // fallthrough
      }
    }

    case PARSING_KEY: {
      if (tok.typeno != Token::STRING_LITERAL) {
        fmt_error(error, Error::PARSE_UNEXPECTED_TOKEN, tok.location)
            << "Expected a string literal (key) but got " << tok.spelling;
        return -1;
      }

      if (!group_stack_.size()) {
        fmt_error(error, Error::INTERNAL_ERROR, tok.location)
            << "group_stack_ is empty" << tok.spelling;
        return -1;
      }

      if (group_stack_.back() != Event::OBJECT_BEGIN) {
        fmt_error(error, Error::INTERNAL_ERROR, tok.location)
            << "group_stack_ is not an object" << tok.spelling;
        return -1;
      }

      event->typeno = Event::OBJECT_KEY;
      if (dry_run) {
        return 1;
      }

      state_ = PARSING_COLON;
      return 1;
    }

    case PARSING_COLON: {
      if (tok.typeno != Token::PUNCTUATION || tok.spelling != ":") {
        fmt_error(error, Error::PARSE_UNEXPECTED_TOKEN, tok.location)
            << "Expected a colon (':') but got " << tok.spelling;
        return -1;
      }
      if (dry_run) {
        return 0;
      }

      state_ = PARSING_VALUE;
      return 0;
    }

    case PARSING_CLOSURE: {
      if (tok.typeno != Token::PUNCTUATION) {
        fmt_error(error, Error::PARSE_UNEXPECTED_TOKEN, tok.location)
            << "Expected ']', '}', or ',' but got " << tok.spelling;
        return -1;
      }

      if (!group_stack_.size()) {
        fmt_error(error, Error::INTERNAL_ERROR, tok.location)
            << "group_stack_ is empty" << tok.spelling;
        return -1;
      }

      if (tok.spelling == ",") {
        if (group_stack_.back() == Event::LIST_BEGIN) {
          if (dry_run) {
            return 0;
          }

          state_ = PARSING_VALUE;
          return 0;
        } else if (group_stack_.back() == Event::OBJECT_BEGIN) {
          if (dry_run) {
            return 0;
          }
          state_ = PARSING_KEY;
          return 0;
        } else {
          fmt_error(error, Error::INTERNAL_ERROR, tok.location)
              << "Top of group stack is not a list or object" << tok.spelling;
          return -1;
        }
      }

      if (group_stack_.back() == Event::LIST_BEGIN) {
        if (tok.spelling != "]") {
          fmt_error(error, Error::PARSE_UNEXPECTED_TOKEN, tok.location)
              << "Expected ']' but got " << tok.spelling;
          return -1;
        }

        event->typeno = Event::LIST_END;
        if (dry_run) {
          return 1;
        }
        group_stack_.pop_back();
        state_ = PARSING_CLOSURE;
        return 1;
      }

      if (group_stack_.back() == Event::OBJECT_BEGIN) {
        if (tok.spelling != "}") {
          fmt_error(error, Error::PARSE_UNEXPECTED_TOKEN, tok.location)
              << "Expected '}' but got " << tok.spelling;
          return -1;
        }

        event->typeno = Event::OBJECT_END;
        if (dry_run) {
          return 1;
        }
        group_stack_.pop_back();
        return 1;
      }

      fmt_error(error, Error::PARSE_UNEXPECTED_TOKEN, tok.location)
          << "Expected ']', '}', or ',' but got " << tok.spelling;
      return -1;
    }

    case PARSING_ERROR: {
      fmt_error(error, Error::PARSE_BAD_STATE, tok.location)
          << "Parser is in an error state";
      return -1;
    }

    default: {
      fmt_error(error, Error::INTERNAL_ERROR, tok.location)
          << "Unknown parser state: " << state_ << tok.spelling;
      return -1;
    }
  }

  return 0;
}

// -----------------------------------------------------------------------------
//    LexerParser
// -----------------------------------------------------------------------------

int LexerParser::init(Error* error) {
  return scanner_.init(error);
}

int LexerParser::begin(const re2::StringPiece& string) {
  parser_.reset();
  return scanner_.begin(string);
}

int LexerParser::get_next_event(Event* event, Error* error) {
  int result = 0;
  while (result == 0) {
    result = scanner_.pump(&token_, error);
    if (result < 0) {
      return result;
    }

    result = parser_.handle_token(token_, event, error);
    if (result < 0) {
      return result;
    }
    if (result > 0) {
      return 0;
    }
    // If result == 0 then the token did not instigate an event
  }
  return result;
}

int LexerParser::peek_next_event(Event* event, Error* error) {
  int result = 0;
  while (result == 0) {
    result = scanner_.peek(&token_, error);
    if (result < 0) {
      return result;
    }

    result = parser_.handle_token(token_, event, error, /*dry_run=*/true);
    if (result < 0) {
      return result;
    }
    if (result > 0) {
      return 0;
    }

    // If result == 0 then the token did not instigate an event. Since it was
    // non-event producing, we can advance the stream.
    scanner_.pump(&token_, error);
    parser_.handle_token(token_, event, error, /*dry_run=*/false);
  }
  return 0;
}

// -----------------------------------------------------------------------------
//    High Level Parse Functinos
// -----------------------------------------------------------------------------

int parse(const re2::StringPiece& source, Event* buf, size_t n, Error* error) {
  LexerParser parser{};

  if (parser.init(error)) {
    return -1;
  }
  if (parser.begin(source)) {
    return -2;
  }

  Error local_error;
  if (!error) {
    error = &local_error;
  }

  size_t nevents = 0;
  for (; nevents < n; ++nevents) {
    int result = parser.get_next_event(&buf[nevents], error);
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
    int result = parser.get_next_event(&local_event, error);
    if (result < 0) {
      if (error->code == Error::LEX_INPUT_FINISHED) {
        return nevents;
      } else {
        return -5;
      }
    }
  }
}

int verify(const re2::StringPiece& source, Error* error) {
  if (parse(source, nullptr, 0, error) >= 0) {
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

util::FixedBufStream<char> fmt_error(Error* error, Error::Code code,
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
