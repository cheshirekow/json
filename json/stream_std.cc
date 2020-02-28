// Copyright 2018 Josh Bialkowski <josh.bialkowski@gmail.com>
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <glog/logging.h>

#include "json/stream_std.h"

namespace json {
namespace stream {

// -----------------------------------------------------------------------------
//    Parser Helpers
// -----------------------------------------------------------------------------

void ParseString(const Token& token, std::string* value) {
  if (token.typeno == json::Token::STRING_LITERAL) {
    *value = std::string(token.spelling.begin() + 1, token.spelling.size() - 2);
  } else {
    *value = std::string(token.spelling.begin(), token.spelling.size());
  }
}

// -----------------------------------------------------------------------------
//    Parser Overloads
// -----------------------------------------------------------------------------

void ParseValue(const Event& event, LexerParser* stream, std::string* value) {
  switch (event.typeno) {
    case json::Event::VALUE_LITERAL:
    case json::Event::OBJECT_KEY:
      break;
    default:
      LOG(WARNING) << fmt::format(
          "Cannot parse JSON event {} as a string value at {}:{}",
          json::Event::ToString(event.typeno), event.token.location.lineno,
          event.token.location.colno);
  }
  ParseString(event.token, value);
}

// -----------------------------------------------------------------------------
//    Emitter Helpers
// -----------------------------------------------------------------------------

void EmitString(const std::string& str, BufPrinter* out) {
  (*out)("\"%s\"", str.c_str());
}

// -----------------------------------------------------------------------------
//    Emitter Overloads
// -----------------------------------------------------------------------------

void EmitValue(const std::string& value, const SerializeOpts& opts,
               size_t depth, BufPrinter* out) {
  EmitString(value, out);
}

}  // namespace stream
}  // namespace json
