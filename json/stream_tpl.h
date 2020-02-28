#pragma once
// Copyright 2018 Josh Bialkowski <josh.bialkowski@gmail.com>

#include <fmt/format.h>
#include <fmt/ostream.h>
#include <glog/logging.h>

#include "json/stream.h"

namespace json {
namespace stream {

// -----------------------------------------------------------------------------
//    High Level
// -----------------------------------------------------------------------------

template <typename T>
void Parse(json::LexerParser* stream, T* out) {
  Event event;
  Error error;
  if (stream->GetNextEvent(&event, &error) != 0) {
    LOG(ERROR) << "Invalid JSON: " << error.msg;
    return;
  }
  ParseValue(event, stream, out);
}

template <typename T>
void Parse(const re2::StringPiece& content, T* out) {
  json::LexerParser stream{};
  json::Error error;
  CHECK_EQ(0, stream.Init(&error)) << error.msg;
  CHECK_EQ(0, stream.Begin(content));
  Parse(&stream, out);
}

// -----------------------------------------------------------------------------
//    Parser Helpers
// -----------------------------------------------------------------------------

template <typename T>
void ParseInteger(const Token& token, T* obj) {
  if (token.typeno != Token::NUMERIC_LITERAL) {
    printf("WARNING, Can't parse token of type '%d' as an integer\n",
           token.typeno);
  }
  if (sizeof(T) == 1) {
    int16_t temp;
    if (RE2::FullMatch(token.spelling, "(.+)", &temp)) {
      *obj = temp;
    } else {
      printf("WARNING, re2 can't parse token '%s' as an integer\n",
             token.spelling.as_string().c_str());
    }
  } else {
    if (RE2::FullMatch(token.spelling, "(.+)", obj)) {
      // OK
    } else {
      printf("WARNING, re2 can't parse token '%s' as an integer\n",
             token.spelling.as_string().c_str());
    }
  }
}

template <typename T>
void ParseRealNumber(const Token& token, T* obj) {
  if (token.typeno != Token::NUMERIC_LITERAL) {
    printf("WARNING, Can't parse token of type '%d' as a real number\n",
           token.typeno);
  }
  if (RE2::FullMatch(token.spelling, "(.+)", obj)) {
    // OK
  } else {
    printf("WARNING, re2 can't parse token '%s' as a real number\n",
           token.spelling.as_string().c_str());
  }
}

template <size_t N>
void ParseString(const Token& token, char (*buf)[N]) {
  // NOTE(josh): strip literal quotes from beginning/end of the string.
  token.spelling.substr(1, token.spelling.size() - 2).copy(*buf, N, 0);
}

template <class T, size_t N>
void ParseArray(LexerParser* stream, T (*arr)[N]) {
  T* iter = &(*arr)[0];
  Event event{};
  while (stream->GetNextEvent(&event) == 0) {
    switch (event.typeno) {
      case Event::LIST_BEGIN:
        break;

      case Event::LIST_END:
        return;

      default:
        if (iter < &(*arr)[N]) {
          ParseValue(event, stream, iter);
          iter++;
        } else {
          printf("WARNING: skipping elements after %d", static_cast<int>(N));
        }
        break;
    }
  }
}

template <typename T>
void ParseObject(const Event& entry_event, LexerParser* stream, T* out) {
  if (entry_event.typeno != json::Event::OBJECT_BEGIN) {
    LOG(WARNING) << fmt::format(
        "Expected JSON object for {}, but instead got {} at {}:{}",
        __PRETTY_FUNCTION__, json::Event::ToString(entry_event.typeno),
        entry_event.token.location.lineno, entry_event.token.location.colno);
    SinkValue(entry_event, stream);
    return;
  }

  json::Event event;
  json::Error error;
  while (stream->GetNextEvent(&event, &error) == 0) {
    if (event.typeno == json::Event::OBJECT_END) {
      return;
    }

    if (event.typeno != json::Event::OBJECT_KEY) {
      LOG(WARNING) << fmt::format(
          "{}:{} Unexpected {} event at {}:{}", __PRETTY_FUNCTION__, __LINE__,
          json::Event::ToString(event.typeno), event.token.location.lineno,
          event.token.location.colno);
      return;
    }

    // event.typeno == OBJECT_KEY, as expected
    Token keytoken = event.token;

    // NOTE(josh): strip literal quotes off of string token
    re2::StringPiece keyvalue =
        keytoken.spelling.substr(1, keytoken.spelling.size() - 2);

    if (stream->GetNextEvent(&event, &error) != 0) {
      LOG(WARNING) << error.msg;
      return;
    }

    if (ParseField(keyvalue, event, stream, out)) {
      LOG(WARNING) << fmt::format(
          "Unrecognized key {}({}) at {}:{}", keyvalue, RuntimeHash(keyvalue),
          keytoken.location.lineno, keytoken.location.colno);
    }
  }
  LOG(WARNING) << error.msg;
}

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

// -----------------------------------------------------------------------------
//    Emitter helpers
// -----------------------------------------------------------------------------

template <typename T>
void EmitInteger(T value, BufPrinter* out) {
  (*out)("%ld", static_cast<long>(value));  // NOLINT(runtime/int)
}

template <typename T>
void EmitRealNumber(T value, BufPrinter* out) {
  (*out)("%f", static_cast<double>(value));
}

template <size_t N>
void EmitString(const char (&arr)[N], BufPrinter* out) {
  (*out)("\"%s\"", arr);
}

template <typename T, size_t N>
void EmitValue(const T (&arr)[N], const SerializeOpts& opts, size_t depth,
               BufPrinter* out) {
  EmitArray(arr, opts, depth, out);
}

template <class T, size_t N>
void EmitArray(const T (&arr)[N], const SerializeOpts& opts, size_t depth,
               BufPrinter* out) {
  (*out)("[");
  if (opts.indent) {
    (*out)("\n");
  }
  const T* iter = &arr[0];
  while (iter < &arr[N]) {
    FmtIndent(out, opts.indent, depth + 1);
    EmitValue(*iter, opts, depth + 1, out);
    ++iter;
    if (iter < &arr[N]) {
      (*out)("%s", opts.separators[1]);
    }
    if (opts.indent) {
      (*out)("\n");
    }
  }
  (*out)("]");
}

template <typename T>
void EmitIterable(const T& obj, const SerializeOpts& opts, size_t depth,
                  BufPrinter* out) {
  if (obj.size() < 1) {
    (*out)("[]");
  } else {
    (*out)("[");
    if (opts.indent) {
      (*out)("\n");
    }
    auto iter = obj.begin();
    while (iter != obj.end()) {
      FmtIndent(out, opts.indent, depth + 1);
      EmitValue(*iter, opts, depth + 1, out);
      ++iter;
      if (iter != obj.end()) {
        (*out)("%s", opts.separators[1]);
      }
      if (opts.indent) {
        (*out)("\n");
      }
    }
    FmtIndent(out, opts.indent, depth);
    (*out)("]");
  }
}

template <typename T>
void EmitField(const char* key, const T& value, const SerializeOpts& opts,
               size_t depth, BufPrinter* out) {
  FmtIndent(out, opts.indent, depth + 1);
  (*out)("\"%s\"%s", key, opts.separators[0]);
  EmitValue(value, opts, depth + 1, out);
}

// -----------------------------------------------------------------------------
//    Emitter Overloads
// -----------------------------------------------------------------------------

template <size_t N>
void EmitValue(const char (&value)[N], const SerializeOpts& opts, size_t depth,
               BufPrinter* out) {
  EmitString(value, out);
}

}  // namespace stream
}  // namespace json
