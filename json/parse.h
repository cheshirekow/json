#pragma once
// Copyright 2018 Josh Bialkowski <josh.bialkowski@gmail.com>

#include <fmt/format.h>
#include <fmt/ostream.h>
#include <glog/logging.h>

#include "json/json.h"
#include "json/util.h"

namespace json {

// -----------------------------------------------------------------------------
//    Parse Helpers
// -----------------------------------------------------------------------------
// These implement reusable algorithms that are used for multiple different
// types

template <typename T>
void ParseInteger(const Token& token, T* value);

template <typename T>
void ParseRealNumber(const Token& token, T* value);

void ParseBoolean(const Token& token, bool* value);

template <size_t N>
void ParseString(const Token& token, char (*str)[N]);

// -----------------------------------------------------------------------------
//    Sink Functions
// -----------------------------------------------------------------------------
// Use these to consume a coherent event-group from the LexerParser without
// actually looking at the data.

// Consume a value, ignoring it's contents
void SinkValue(const Event& event, LexerParser* stream);

void SinkValue(LexerParser* stream);

// Consume an object, ignoring it's contents
void SinkObject(LexerParser* stream);

// Consume a list, ignoring it's contents
void SinkList(LexerParser* stream);

}  // namespace json

namespace json {
namespace stream {

// -----------------------------------------------------------------------------
//    High level API
// -----------------------------------------------------------------------------

// Entry point template, pops the first event and then calls the appropriate
// Parse() overload.
template <typename T>
void Parse(json::LexerParser* stream, T* out);

// Convenience entry point. Parse the input string into the JSON-stream
// serializable object
template <typename T>
void Parse(const re2::StringPiece& str, T* obj);

// -----------------------------------------------------------------------------
//    Parse Helpers
// -----------------------------------------------------------------------------

// Parse an homogenous array of elements.
template <class T, size_t N>
void ParseArray(LexerParser* stream, T (*arr)[N]);

// Helper template for implementing object parsers. Implements all of the common
// functionality. Requires the existence of an overload in the form
//
//    int ParseField(const re2::StringPiece& key, const Event& event,
//                   LexerParser* stream, T* out)
//
// which it will call for each field key discovered in the object
template <typename T>
void ParseObject(const Event& entry_event, json::LexerParser* stream, T* out);

// -----------------------------------------------------------------------------
//    Value Parser Overloads
// -----------------------------------------------------------------------------
// These are parse functions for supported types
// NOTE(josh): In general a parse function accepts only one class of input
// (i.e. scalar, object, or list) and one token type (string literal, numeric
// literal). If another type is encountered, a warning will be printed and
// the value will be sink'ed

void ParseValue(const Event& event, LexerParser* stream, int8_t* value);
void ParseValue(const Event& event, LexerParser* stream, int16_t* value);
void ParseValue(const Event& event, LexerParser* stream, int32_t* value);
void ParseValue(const Event& event, LexerParser* stream, int64_t* value);
void ParseValue(const Event& event, LexerParser* stream, uint8_t* value);
void ParseValue(const Event& event, LexerParser* stream, uint16_t* value);
void ParseValue(const Event& event, LexerParser* stream, uint32_t* value);
void ParseValue(const Event& event, LexerParser* stream, uint64_t* value);
void ParseValue(const Event& event, LexerParser* stream, double* value);
void ParseValue(const Event& event, LexerParser* stream, float* value);
void ParseValue(const Event& event, LexerParser* stream, bool* value);

template <size_t N>
void ParseValue(const Event& event, LexerParser* stream, char (*str)[N]);

template <typename T, size_t N>
void ParseValue(const Event& event, LexerParser* stream, T (*arr)[N]);

}  // namespace stream
}  // namespace json

//
//
//
// -----------------------------------------------------------------------------
//    Template Implementations
// -----------------------------------------------------------------------------
//
//
//

namespace json {

// -----------------------------------------------------------------------------
//    Parser Helpers
// -----------------------------------------------------------------------------

template <typename T>
void ParseInteger(const Token& token, T* obj) {
  if (token.typeno != Token::NUMERIC_LITERAL) {
    LOG(WARNING) << fmt::format("Can't parse token of type '{}' as an integer",
                                token.typeno);
  }
  if (sizeof(T) == 1) {
    int16_t temp;
    if (RE2::FullMatch(token.spelling, "(.+)", &temp)) {
      *obj = temp;
    } else {
      LOG(WARNING) << fmt::format("re2 can't parse token '{}' as an integer",
                                  token.spelling.as_string());
    }
  } else {
    if (RE2::FullMatch(token.spelling, "(.+)", obj)) {
      // OK
    } else {
      LOG(WARNING) << fmt::format("re2 can't parse token '{}' as an integer",
                                  token.spelling.as_string());
    }
  }
}

template <typename T>
void ParseRealNumber(const Token& token, T* obj) {
  if (token.typeno != Token::NUMERIC_LITERAL) {
    LOG(WARNING) << fmt::format(
        "Can't parse token of type '{}' as a real number", token.typeno);
  }
  if (RE2::FullMatch(token.spelling, "(.+)", obj)) {
    // OK
  } else {
    LOG(WARNING) << fmt::format("re2 can't parse token '{}' as a real number",
                                token.spelling.as_string());
  }
}

template <size_t N>
void ParseString(const Token& token, char (*buf)[N]) {
  // NOTE(josh): strip literal quotes from beginning/end of the string.
  re2::StringPiece unquoted =
      token.spelling.substr(1, token.spelling.size() - 2);
  unescape(unquoted, *buf, (*buf) + N);
}
}  // namespace json

namespace json {
namespace stream {

// -----------------------------------------------------------------------------
//    Parse Overloads
// -----------------------------------------------------------------------------

template <size_t N>
void ParseValue(const Event& event, LexerParser* stream, char (*str)[N]) {
  ParseString(event.token, str);
}

template <typename T, size_t N>
void ParseValue(const Event& event, LexerParser* stream, T (*arr)[N]) {
  assert(event.typeno == Event::LIST_BEGIN);
  ParseArray(stream, arr);
}

// template <typename T>
// void Parse(const Event& event, LexerParser* stream, T* obj) {
//   assert(event.typeno == Event::OBJECT_BEGIN);
//   ParseObject(stream, obj);
// }

}  // namespace stream
}  // namespace json
