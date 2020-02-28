#pragma once
// Copyright 2018 Josh Bialkowski <josh.bialkowski@gmail.com>

#include <cassert>

#include "json/json.h"

namespace json {

// constexpr string, used to implement tagging with strings
/*
 *  via Scott Schurr's C++ Now 2012 presentation
 */
class Tag {
 public:
  template <uint32_t N>
  // Construct from a string literal
  constexpr Tag(const char (&str)[N]) : ptr_(str), size_(N - 1) {}
  constexpr Tag(const char* str, size_t len) : ptr_(str), size_(len) {}
  explicit Tag(const re2::StringPiece& str)
      : ptr_(str.begin()), size_(str.size()) {}

  // return the character at index i
  constexpr char operator[](uint32_t i) const {
    // if we care about overflow
    // return n < size_ ? ptr_[i] : throw std::out_of_range("");
    return ptr_[i];
  }

  // return the number of characters in the string, including the terminal null
  constexpr uint32_t size() const {
    return size_;
  }

  // Donald Knuth's hash function
  // TODO(josh): validate or replace this hash function
  constexpr uint64_t Hash(int i, uint64_t hash) const {
    return i == size() ? hash
                       : Hash(i + 1, ((hash << 5) ^ (hash >> 27)) ^ ptr_[i]);
  }

  // return a hash of the string
  constexpr uint64_t Hash() const {
    return Hash(0, size());
  }

 private:
  const char* const ptr_;
  const uint32_t size_;
};

// Return an unsigned 64bit hash of @p string
template <uint32_t N>
inline constexpr uint64_t Hash(const char (&str)[N]) {
  return Tag(str).Hash();
}

// Donald Knuth's hash function
// TODO(josh): validate or replace this hash function
// NOTE(josh): this is the same as above, but iterative instead of stack-based
inline uint64_t RuntimeHash(const re2::StringPiece& str) {
  uint64_t hash = str.size();
  for (size_t idx = 0; idx < str.size(); ++idx) {
    hash = ((hash << 5) ^ (hash >> 27)) ^ str[idx];
  }
  return hash;
}

// Streaming API for creating json-serializable structures
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

// Convenience entry point. Serialize the JSON-stream serializable object
// into the provided character buffer
template <typename T>
void Emit(const T& obj, const SerializeOpts& opts, char* begin, char* end);

// -----------------------------------------------------------------------------
//    Sink Functions
// -----------------------------------------------------------------------------

// Consume a value, ignoring it's contents
void SinkValue(const Event& event, LexerParser* stream);

// Consume an object, ignoring it's contents
void SinkObject(LexerParser* stream);

// Consume a list, ignoring it's contents
void SinkList(LexerParser* stream);

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

// -----------------------------------------------------------------------------
//    Emit Helpers
// -----------------------------------------------------------------------------
// These implement reusable algorithms that are used for multiple different
// types

template <typename T>
void EmitInteger(T value, BufPrinter* out);

template <typename T>
void EmitRealNumber(T value, BufPrinter* out);

void EmitBoolean(bool value, BufPrinter* out);

template <size_t N>
void EmitString(size_t depth, BufPrinter* out);

template <class T, size_t N>
void EmitArray(const T (&arr)[N], const SerializeOpts& opts, size_t depth,
               BufPrinter* out);

// Emit objects from any container that implements the standard iterable
// concept (i.e. `.begin()` and `.end()` return iterator of some type that
// deferences to `::value_type`.
template <typename T>
void EmitIterable(const T& obj, const SerializeOpts& opts, size_t depth,
                  BufPrinter* out);

// Template to emit a field in an object map
template <typename T>
void EmitField(const char* key, const T& value, const SerializeOpts& opts,
               size_t depth, BufPrinter* out);

// Emit a separator between two consecutive fields. This generally includes a
// comma and possibly a newline.
void EmitFieldSep(const SerializeOpts& opts, BufPrinter* out);

// -----------------------------------------------------------------------------
//    Emit Value Overloads
// -----------------------------------------------------------------------------

void EmitValue(int8_t value, const SerializeOpts& opts, size_t depth,
               BufPrinter* out);
void EmitValue(int16_t value, const SerializeOpts& opts, size_t depth,
               BufPrinter* out);
void EmitValue(int32_t value, const SerializeOpts& opts, size_t depth,
               BufPrinter* out);
void EmitValue(int64_t value, const SerializeOpts& opts, size_t depth,
               BufPrinter* out);
void EmitValue(uint8_t value, const SerializeOpts& opts, size_t depth,
               BufPrinter* out);
void EmitValue(uint16_t value, const SerializeOpts& opts, size_t depth,
               BufPrinter* out);
void EmitValue(uint32_t value, const SerializeOpts& opts, size_t depth,
               BufPrinter* out);
void EmitValue(uint64_t value, const SerializeOpts& opts, size_t depth,
               BufPrinter* out);
void EmitValue(float value, const SerializeOpts& opts, size_t depth,
               BufPrinter* out);
void EmitValue(double value, const SerializeOpts& opts, size_t depth,
               BufPrinter* out);
void EmitValue(bool value, const SerializeOpts& opts, size_t depth,
               BufPrinter* out);

template <size_t N>
void EmitValue(const char (&value)[N], const SerializeOpts& opts, size_t depth,
               BufPrinter* out);

template <typename T, size_t N>
void EmitValue(const T (&arr)[N], const SerializeOpts& opts, size_t depth,
               BufPrinter* out);

}  // namespace stream
}  // namespace json
