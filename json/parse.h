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
