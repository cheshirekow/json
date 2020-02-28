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
int ParseInteger(const Token& token, T* value);

template <typename T>
int ParseRealNumber(const Token& token, T* value);

int ParseBoolean(const Token& token, bool* value);

template <size_t N>
int ParseString(const Token& token, char (*str)[N]);

// -----------------------------------------------------------------------------
//    Sink Functions
// -----------------------------------------------------------------------------
// Use these to consume a coherent event-group from the LexerParser without
// actually looking at the data.

// Consume a value, ignoring it's contents
int SinkValue(const Event& event, LexerParser* stream);

int SinkValue(LexerParser* stream);

// Consume an object, ignoring it's contents
int SinkObject(LexerParser* stream);

// Consume a list, ignoring it's contents
int SinkList(LexerParser* stream);

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
int ParseInteger(const Token& token, T* obj) {
  if (token.typeno != Token::NUMERIC_LITERAL) {
    LOG(WARNING) << fmt::format("Can't parse token of type '{}' as an integer",
                                token.typeno);
    return 1;
  }
  if (sizeof(T) == 1) {
    // RE2 doesn't support parsing into an 8-bit integer. It will try to parse
    // it as a ASCII char instead of a numeric value.
    int16_t temp;
    if (RE2::FullMatch(token.spelling, "(.+)", &temp)) {
      *obj = static_cast<T>(temp);
      return 0;
    } else {
      LOG(WARNING) << fmt::format("re2 can't parse token '{}' as an integer",
                                  token.spelling.as_string());
    }
  } else {
    if (RE2::FullMatch(token.spelling, "(.+)", obj)) {
      return 0;
    } else {
      LOG(WARNING) << fmt::format("re2 can't parse token '{}' as an integer",
                                  token.spelling.as_string());
    }
  }
  return 1;
}

template <typename T>
int ParseRealNumber(const Token& token, T* obj) {
  if (token.typeno != Token::NUMERIC_LITERAL) {
    LOG(WARNING) << fmt::format(
        "Can't parse token of type '{}' as a real number", token.typeno);
    return 1;
  }
  if (RE2::FullMatch(token.spelling, "(.+)", obj)) {
    return 0;
  } else {
    LOG(WARNING) << fmt::format("re2 can't parse token '{}' as a real number",
                                token.spelling.as_string());
  }
  return 1;
}

template <size_t N>
int ParseString(const Token& token, char (*buf)[N]) {
  // NOTE(josh): strip literal quotes from beginning/end of the string.
  re2::StringPiece unquoted =
      token.spelling.substr(1, token.spelling.size() - 2);
  unescape(unquoted, *buf, (*buf) + N);
  return 0;
}

}  // namespace json
