// Copyright 2018 Josh Bialkowski <josh.bialkowski@gmail.com>
#include "json/stream.h"

// NOTE(josh): extern in stream.h
// It would be really nice if we didn't need a global object for this. Is there
// any globally defined extern variable in C that we could use?
json::stream::MacroPad kMacroPad{};

namespace json {
namespace stream {

// -----------------------------------------------------------------------------
//    Parser Implementations
// -----------------------------------------------------------------------------

void ParseBoolean(const Token& token, bool* out) {
  if (token.spelling == "true") {
    (*out) = true;
  } else if (token.spelling == "false") {
    (*out) = false;
  } else {
    printf("WARNING");
  }
}

// -----------------------------------------------------------------------------
//    Parse Overloads
// -----------------------------------------------------------------------------

void Parse(const Event& event, LexerParser* stream, int8_t* value) {
  ParseInteger(event.token, value);
}

void Parse(const Event& event, LexerParser* stream, int16_t* value) {
  ParseInteger(event.token, value);
}

void Parse(const Event& event, LexerParser* stream, int32_t* value) {
  ParseInteger(event.token, value);
}

void Parse(const Event& event, LexerParser* stream, int64_t* value) {
  ParseInteger(event.token, value);
}

void Parse(const Event& event, LexerParser* stream, uint8_t* value) {
  ParseInteger(event.token, value);
}

void Parse(const Event& event, LexerParser* stream, uint16_t* value) {
  ParseInteger(event.token, value);
}

void Parse(const Event& event, LexerParser* stream, uint32_t* value) {
  ParseInteger(event.token, value);
}

void Parse(const Event& event, LexerParser* stream, uint64_t* value) {
  ParseInteger(event.token, value);
}

void Parse(const Event& event, LexerParser* stream, double* value) {
  ParseRealNumber(event.token, value);
}

void Parse(const Event& event, LexerParser* stream, float* value) {
  ParseRealNumber(event.token, value);
}

void Parse(const Event& event, LexerParser* stream, bool* value) {
  ParseBoolean(event.token, value);
}

// -----------------------------------------------------------------------------
//    Emitter Implementations
// -----------------------------------------------------------------------------

void EmitBoolean(bool value, BufPrinter* out) {
  if (value) {
    (*out)("true");
  } else {
    (*out)("false");
  }
}

// -----------------------------------------------------------------------------
//    Emitter Overloads
// -----------------------------------------------------------------------------

void Emit(int8_t value, const SerializeOpts& opts, size_t depth,
          BufPrinter* out) {
  EmitInteger(value, out);
}

void Emit(int16_t value, const SerializeOpts& opts, size_t depth,
          BufPrinter* out) {
  EmitInteger(value, out);
}

void Emit(int32_t value, const SerializeOpts& opts, size_t depth,
          BufPrinter* out) {
  EmitInteger(value, out);
}

void Emit(int64_t value, const SerializeOpts& opts, size_t depth,
          BufPrinter* out) {
  EmitInteger(value, out);
}

void Emit(uint8_t value, const SerializeOpts& opts, size_t depth,
          BufPrinter* out) {
  EmitInteger(value, out);
}

void Emit(uint16_t value, const SerializeOpts& opts, size_t depth,
          BufPrinter* out) {
  EmitInteger(value, out);
}

void Emit(uint32_t value, const SerializeOpts& opts, size_t depth,
          BufPrinter* out) {
  EmitInteger(value, out);
}

void Emit(uint64_t value, const SerializeOpts& opts, size_t depth,
          BufPrinter* out) {
  EmitInteger(value, out);
}

void Emit(float value, const SerializeOpts& opts, size_t depth,
          BufPrinter* out) {
  EmitRealNumber(value, out);
}

void Emit(double value, const SerializeOpts& opts, size_t depth,
          BufPrinter* out) {
  EmitRealNumber(value, out);
}

void Emit(bool value, const SerializeOpts& opts, size_t depth,
          BufPrinter* out) {
  EmitBoolean(value, out);
}

}  // namespace stream
}  // namespace json
