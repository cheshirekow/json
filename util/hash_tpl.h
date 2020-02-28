#pragma once
// Copyright 2018 Josh Bialkowski <josh.bialkowski@gmail.com>

#include <cstdint>
namespace hash {
// constexpr string, used to implement tagging with strings
/*
 *  via Scott Schurr's C++ Now 2012 presentation
 */
class Tag {
 public:
  template <uint32_t N>
  // Construct from a string literal
  constexpr Tag(const char (&str)[N]) : ptr_(str), size_(N - 1) {}  // NOLINT
  constexpr Tag(const char* str, uint32_t len) : ptr_(str), size_(len) {}

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
    return static_cast<uint32_t>(i) == size()
               ? hash
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

}  // namespace hash

// Return an unsigned 64bit hash of @p string
template <uint32_t N>
inline constexpr uint64_t Hash(const char (&str)[N]) {
  return hash::Tag(str).Hash();
}
