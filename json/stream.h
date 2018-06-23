#pragma once
// Copyright 2018 Josh Bialkowski <josh.bialkowski@gmail.com>

#include <cassert>
#include <map>
#include <string>

#include "json/json.h"

namespace json {

// Streaming API for creating json-serializable structures
namespace stream {

// Sentinel type used to pad JSON_STREAM macros
struct MacroPad {};

// -----------------------------------------------------------------------------
//    High level API
// -----------------------------------------------------------------------------

// Convenience entry point. Parse the input string into the JSON-stream
// serializable object
template <typename T>
void Parse(const re2::StringPiece& str, T* obj);

// Convenience entry point. Serialize the JSON-stream serializable object
// into the provided character buffer
template <typename T>
void Emit(const T& obj, const SerializeOpts& opts, char* begin, char* end);

// -----------------------------------------------------------------------------
//    Parser Templates
// -----------------------------------------------------------------------------

template <typename T>
void ParseInteger(const Token& token, T* value);

template <typename T>
void ParseRealNumber(const Token& token, T* value);

void ParseBoolean(const Token& token, bool* value);

template <size_t N>
void ParseString(const Token& token, char (*str)[N]);

template <class T, size_t N>
void ParseArray(LexerParser* stream, T (*arr)[N]);

template <typename T>
void ParseObject(LexerParser* stream, T* value);

// -----------------------------------------------------------------------------
//    Parse Overloads
// -----------------------------------------------------------------------------

void Parse(const Event& event, LexerParser* stream, int8_t* value);
void Parse(const Event& event, LexerParser* stream, int16_t* value);
void Parse(const Event& event, LexerParser* stream, int32_t* value);
void Parse(const Event& event, LexerParser* stream, int64_t* value);
void Parse(const Event& event, LexerParser* stream, uint8_t* value);
void Parse(const Event& event, LexerParser* stream, uint16_t* value);
void Parse(const Event& event, LexerParser* stream, uint32_t* value);
void Parse(const Event& event, LexerParser* stream, uint64_t* value);
void Parse(const Event& event, LexerParser* stream, double* value);
void Parse(const Event& event, LexerParser* stream, float* value);
void Parse(const Event& event, LexerParser* stream, bool* value);

template <size_t N>
void Parse(const Event& event, LexerParser* stream, char (*str)[N]);

template <typename T, size_t N>
void Parse(const Event& event, LexerParser* stream, T (*arr)[N]);

// Default implementation is for a type serializable to a JSON-object
// utilizing the streaming API.
template <typename T>
void Parse(const Event& event, LexerParser* stream, T* obj);

// -----------------------------------------------------------------------------
//    Parser indirection
// -----------------------------------------------------------------------------

// Simple template which upcasts type-elided pointer before dispatching the
// parse overload.
template <typename T>
struct ParseHolder {
  static void Dispatch(const Event& event, LexerParser* stream, void* obj) {
    Parse(event, stream, static_cast<T*>(obj));
  }
};

// Type-elided container to hold a value pointer along with it's associated
// parse function pointer.
struct ValueParser {
  void Dispatch(const Event& event, LexerParser* stream) const {
    (*fn_)(event, stream, obj_);
  }

  void* obj_;
  void (*fn_)(const Event&, LexerParser*, void*);
};

template <typename T>
void AssignParser(ValueParser* parser, T* value) {
  parser->obj_ = value;
  parser->fn_ = &ParseHolder<T>::Dispatch;
}

// TODO(josh): Use some fixed storage associated with the parser to store
// these maps, rather than heap-allocating all of them.
typedef std::map<std::string, ValueParser> ParseMap;

// Fill a parse map. Base Case: finish recursion
inline void FillMap(ParseMap* pmap) {}

// Fill a parse map. Base Case: only padding entries are left, so we can
// terminate early
template <typename... Tail>
inline void FillMap(ParseMap* pmap, const char* head_key, MacroPad* head_value,
                    Tail... tail) {}

// Fill a parse map. Recursive Case: assign a parser for the key, value at
// the head of the argument list, then recurse with the remaining arguments.
template <typename Head, typename... Tail>
void FillMap(ParseMap* pmap, const char* head_key, Head* head_value,
             Tail... tail) {
  AssignParser(&((*pmap)[head_key]), head_value);
  FillMap(pmap, tail...);
}

// -----------------------------------------------------------------------------
//    Emitter Templates
// -----------------------------------------------------------------------------

template <typename T>
void EmitInteger(T value, BufPrinter* out);

template <typename T>
void EmitRealNumber(T value, BufPrinter* out);

void EmitBoolean(bool value, BufPrinter* out);

template <size_t N>
void EmitString(size_t depth, BufPrinter* out);

template <class T, size_t N>
void EmitArray(const T (*arr)[N], const SerializeOpts& opts, size_t depth,
               BufPrinter* out);

template <typename T>
void EmitObject(const T& obj, const SerializeOpts& opts, size_t depth,
                BufPrinter* out);

// -----------------------------------------------------------------------------
//    Emit Overloads
// -----------------------------------------------------------------------------

void Emit(int8_t value, const SerializeOpts& opts, size_t depth,
          BufPrinter* out);
void Emit(int16_t value, const SerializeOpts& opts, size_t depth,
          BufPrinter* out);
void Emit(int32_t value, const SerializeOpts& opts, size_t depth,
          BufPrinter* out);
void Emit(int64_t value, const SerializeOpts& opts, size_t depth,
          BufPrinter* out);
void Emit(uint8_t value, const SerializeOpts& opts, size_t depth,
          BufPrinter* out);
void Emit(uint16_t value, const SerializeOpts& opts, size_t depth,
          BufPrinter* out);
void Emit(uint32_t value, const SerializeOpts& opts, size_t depth,
          BufPrinter* out);
void Emit(uint64_t value, const SerializeOpts& opts, size_t depth,
          BufPrinter* out);
void Emit(float value, const SerializeOpts& opts, size_t depth,
          BufPrinter* out);
void Emit(double value, const SerializeOpts& opts, size_t depth,
          BufPrinter* out);
void Emit(bool value, const SerializeOpts& opts, size_t depth, BufPrinter* out);

template <size_t N>
void Emit(const char (&value)[N], const SerializeOpts& opts, size_t depth,
          BufPrinter* out);

template <typename T, size_t N>
void Emit(const T (&arr)[N], const SerializeOpts& opts, size_t depth,
          BufPrinter* out);

template <typename T>
void Emit(const T& obj, const SerializeOpts& opts, size_t depth,
          BufPrinter* out);

// -----------------------------------------------------------------------------
//    Emitter Indirection
// -----------------------------------------------------------------------------

// Simple template which upcasts type-elided pointer before dispatching the
// emit overload.
template <typename T>
struct EmitHolder {
  static void Dispatch(const void* obj, const SerializeOpts& opts, size_t depth,
                       BufPrinter* out) {
    Emit(*static_cast<const T*>(obj), opts, depth, out);
  }
};

// Type-elided container to hold a value pointer along with it's associated
// format/emit function pointer.
struct ValueEmitter {
  void Dispatch(const SerializeOpts& opts, size_t depth,
                BufPrinter* out) const {
    (*fn_)(obj_, opts, depth, out);
  }

  const void* obj_;
  void (*fn_)(const void*, const SerializeOpts&, size_t, BufPrinter*);
};

// TODO(josh): Use some fixed storage associated with the parser to store
// these maps, rather than heap-allocating all of them.
typedef std::map<std::string, ValueEmitter> EmitMap;

template <typename T>
void AssignEmitter(ValueEmitter* emitter, const T* val) {
  emitter->obj_ = val;
  emitter->fn_ = &EmitHolder<T>::Dispatch;
}

// Fill an emit map. Base Case: finish recursion
inline void FillMap(EmitMap* pmap) {}

// Fill a an map. Base Case: only padding entries are left, so we can
// terminate early
template <typename... Tail>
inline void FillMap(EmitMap* pmap, const char* head_key,
                    const MacroPad* head_value, Tail... tail) {}

// Fill a parse map. Recursive Case: assign an for the key, value at
// the head of the argument list, then recurse with the remaining arguments.
template <typename Head, typename... Tail>
void FillMap(EmitMap* pmap, const char* head_key, const Head* head_value,
             Tail... tail) {
  AssignEmitter(&((*pmap)[head_key]), head_value);
  FillMap(pmap, tail...);
}

// -----------------------------------------------------------------------------
//    Extra stuff
// -----------------------------------------------------------------------------

// Template metaprogram contains a boolean `Value` which is true if the type
// <T> is JSON-stream serializable (i.e. it implements `GetParseMap` and
// `GetEmitMap`).
template <typename T>
struct IsSerializable {
  template <typename U, void (U::*)(ParseMap*)>
  struct ParseSFINAE {};

  template <typename U>
  static int8_t TestParse(ParseSFINAE<U, &U::GetParseMap>*);

  template <typename U>
  static int16_t TestParse(...);

  template <typename U, void (U::*)(EmitMap*) const>
  struct EmitSFINAE {};

  template <typename U>
  static int8_t TestEmit(EmitSFINAE<U, &U::GetEmitMap>*);

  template <typename U>
  static int16_t TestEmit(...);

  static constexpr bool Value = (sizeof(TestParse<T>(0)) == sizeof(int8_t) &&
                                 sizeof(TestEmit<T>(0)) == sizeof(int8_t));
};

}  // namespace stream
}  // namespace json

//
//
//
// -----------------------------------------------------------------------------
//                       Template Implementations
// -----------------------------------------------------------------------------
//
//
//

namespace json {
namespace stream {

// -----------------------------------------------------------------------------
//    High level API
// -----------------------------------------------------------------------------

template <typename T>
void Parse(const re2::StringPiece& string, T* obj) {
  static_assert(IsSerializable<T>::Value,
                "Object type is not JSON-stream serializable");

  LexerParser stream;
  stream.Init(nullptr);
  stream.Begin(string);
  ParseObject(&stream, obj);
}

template <typename T>
void Emit(const T& obj, const SerializeOpts& opts, char* begin, char* end) {
  BufPrinter out{begin, end};
  EmitObject(obj, opts, 0, &out);
}

// -----------------------------------------------------------------------------
//    Parser Implementations
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
  T value{};
  ValueParser handler{};
  AssignParser(&handler, &value);
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
          value = *iter;
          handler.Dispatch(event, stream);
          *(iter++) = value;
        } else {
          printf("WARNING: skipping elements after %d", static_cast<int>(N));
        }
        break;
    }
  }
}

template <typename T>
void ParseObject(LexerParser* stream, T* obj) {
  // TODO(josh): don't use std::map, use some storage from within the `parser`
  // to fill the map.
  ParseMap hmap;
  obj->GetParseMap(&hmap);
  const ValueParser* handler = nullptr;
  Event event{};
  while (stream->GetNextEvent(&event) == 0) {
    if (handler) {
      handler->Dispatch(event, stream);
      handler = nullptr;
      continue;
    }

    switch (event.typeno) {
      case Event::OBJECT_BEGIN:
        break;

      case Event::OBJECT_END:
        return;

      case Event::OBJECT_KEY: {
        // TODO(josh): don't create a heap string with as_string()
        std::string key =
            event.token.spelling.substr(1, event.token.spelling.size() - 2)
                .as_string();
        auto iter = hmap.find(key);
        if (iter == hmap.end()) {
          printf("No Parser for '%s'\n", key.c_str());
          printf("Parsers: ");
          for (const auto& pair : hmap) {
            printf(" '%s',", pair.first.c_str());
          }
          printf("\n");
        } else {
          handler = &(iter->second);
        }
        break;
      }

      default:
        printf("Unhandled object event %d\n", event.typeno);
        break;
    }
  }
}

// -----------------------------------------------------------------------------
//    Parse Overloads
// -----------------------------------------------------------------------------

template <size_t N>
void Parse(const Event& event, LexerParser* stream, char (*str)[N]) {
  ParseString(event.token, str);
}

template <typename T, size_t N>
void Parse(const Event& event, LexerParser* stream, T (*arr)[N]) {
  assert(event.typeno == Event::LIST_BEGIN);
  ParseArray(stream, arr);
}

template <typename T>
void Parse(const Event& event, LexerParser* stream, T* obj) {
  assert(event.typeno == Event::OBJECT_BEGIN);
  ParseObject(stream, obj);
}

// -----------------------------------------------------------------------------
//    Emitter Templates
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

template <class T, size_t N>
void EmitArray(const T (&arr)[N], const SerializeOpts& opts, size_t depth,
               BufPrinter* out) {
  (*out)("[");
  if (opts.indent) {
    (*out)("\n");
  }
  const T* iter = &arr[0];
  while (iter < &arr[N]) {
    ValueEmitter emitter{};
    AssignEmitter(&emitter, &(*iter));
    FmtIndent(out, opts.indent, depth + 1);
    emitter.Dispatch(opts, depth + 1, out);
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
void EmitObject(const T& obj, const SerializeOpts& opts, size_t depth,
                BufPrinter* out) {
  EmitMap emap;
  obj.GetEmitMap(&emap);
  if (emap.size() < 1) {
    (*out)("{}");
  } else {
    (*out)("{");
    if (opts.indent) {
      (*out)("\n");
    }
    auto iter = emap.begin();
    while (iter != emap.end()) {
      FmtIndent(out, opts.indent, depth + 1);
      (*out)("\"%s\"%s", iter->first.c_str(), opts.separators[0]);
      iter->second.Dispatch(opts, depth + 1, out);
      ++iter;
      if (iter != emap.end()) {
        (*out)("%s", opts.separators[1]);
      }
      if (opts.indent) {
        (*out)("\n");
      }
    }
    FmtIndent(out, opts.indent, depth);
    (*out)("}");
  }
}

// -----------------------------------------------------------------------------
//    Emitter Overloads
// -----------------------------------------------------------------------------

template <size_t N>
void Emit(const char (&value)[N], const SerializeOpts& opts, size_t depth,
          BufPrinter* out) {
  EmitString(value, out);
}

template <typename T, size_t N>
void Emit(const T (&arr)[N], const SerializeOpts& opts, size_t depth,
          BufPrinter* out) {
  EmitArray(arr, opts, depth, out);
}

template <typename T>
void Emit(const T& obj, const SerializeOpts& opts, size_t depth,
          BufPrinter* out) {
  EmitObject(obj, opts, depth, out);
}

}  // namespace stream
}  // namespace json

// It would be really nice if we didn't need a global object for this. Is there
// any globally defined extern variable in C that we could use?
extern json::stream::MacroPad kMacroPad;
