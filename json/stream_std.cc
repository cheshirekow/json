// Copyright 2018 Josh Bialkowski <josh.bialkowski@gmail.com>
#include "json/stream_std.h"

namespace json {
namespace stream {

// -----------------------------------------------------------------------------
//    Parser Templates
// -----------------------------------------------------------------------------

void ParseString(const Token& token, std::string* value) {
  token.spelling.CopyToString(value);
}

// -----------------------------------------------------------------------------
//    Parser Overloads
// -----------------------------------------------------------------------------

void Parse(const Event& event, LexerParser* stream, std::string* value) {
  ParseString(event.token, value);
}

// -----------------------------------------------------------------------------
//    Emitter Templates
// -----------------------------------------------------------------------------

void EmitString(const std::string& str, BufPrinter* out) {
  (*out)(str.c_str());
}

// -----------------------------------------------------------------------------
//    Emitter Overloads
// -----------------------------------------------------------------------------

void Emit(const std::string& value, const SerializeOpts& opts, size_t depth,
          BufPrinter* out) {
  EmitString(value, out);
}

}  // namespace stream
}  // namespace json
