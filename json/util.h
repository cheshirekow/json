#pragma once
// Copyright 2019 Josh Bialkowski <josh.bialkowski@gmail.com>

#include <ostream>
#include <string>

#include <re2/stringpiece.h>

namespace json {

// constexpr string, used to implement tagging with strings
/*
 *  via Scott Schurr's C++ Now 2012 presentation
 */
class Tag {
 public:
  template <uint32_t N>
  // Construct from a string literal
  constexpr Tag(const char (&str)[N]) : ptr_(str), size_(N - 1) {}  // NOLINT
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
  constexpr uint64_t hash(int i, uint64_t hashv) const {
    return static_cast<size_t>(i) == size()
               ? hashv
               : hash(i + 1, ((hashv << 5) ^ (hashv >> 27)) ^ ptr_[i]);
  }

  // return a hash of the string
  constexpr uint64_t hash() const {
    return hash(0, size());
  }

 private:
  const char* const ptr_;
  const uint32_t size_;
};

// Return an unsigned 64bit hash of @p string
template <uint32_t N>
inline constexpr uint64_t hash(const char (&str)[N]) {
  return Tag(str).hash();
}

// Donald Knuth's hash function
// TODO(josh): validate or replace this hash function
// NOTE(josh): this is the same as above, but iterative instead of stack-based
inline uint64_t runtime_hash(const re2::StringPiece& str) {
  uint64_t hash = str.size();
  for (size_t idx = 0; idx < str.size(); ++idx) {
    hash = ((hash << 5) ^ (hash >> 27)) ^ str[idx];
  }
  return hash;
}

/// Return true if the given character is a control code
bool is_control_code(char code);

/// Escape any characters that are invalid JSON and construct (in `out`) a
/// valid JSON string literal.
void escape(const re2::StringPiece& piece, std::ostream* out);

/// Escape any characters that are invalid JSON and return a  valid JSON
/// string literal.
std::string escape(const re2::StringPiece& piece);

/// Unescape any JSON encoded control codes, reconstruct in `out` the original
/// string.
void unescape(const re2::StringPiece& piece, std::ostream* out);

/// Unescape any JSON encoded control codes, return the original string
std::string unescape(const re2::StringPiece& piece);

/// Unescape any JSON encoded control codes, reconstruct the original string
/// in the character buffer pointed to by (begin,end). Return the number of
/// characters in the output (not including the null terminator).
size_t unescape(const re2::StringPiece& piece, char* begin, char* end);

/// Unescape any JSON encoded control codes, reconstruct in `buf` the original
/// string
template <size_t N>
void unescape(const re2::StringPiece& piece, char (*buf)[N]) {
  unescape(piece, *buf, N);
}

}  // namespace json
