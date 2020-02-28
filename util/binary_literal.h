// Copyright (C) 2012 Josh Bialkowski (josh.bialkowski@gmail.com)
/**
 *  @file
 *  @date   Sep 14, 2013
 *  @author Josh Bialkowski (josh.bialkowski@gmail.com)
 *  @brief  shamelessly borrowed from
 *          http://akrzemi1.wordpress.com/2012/10/23/user-defined-literals-part-ii/
 */

#pragma once
#include <limits>
#include <stdexcept>

namespace detail {

template <typename T>
constexpr size_t NumberOfBits() {
  static_assert(std::numeric_limits<T>::is_integer, "only integers allowed");

  // from en.cppreference.com: The value of std::numeric_limits<T>::digits is
  // the number of digits in base-radix that can be represented by the type T
  // without change. For integer types, this is the number of bits not counting
  // the sign bit. For floating-point types, this is the number of digits in
  // the mantissa.
  return std::numeric_limits<T>::digits;
}

// compute the length of a string using recursion, so that it can be
// compile-time evaluated
constexpr size_t StringLength(const char* str, size_t current_len = 0) {
  return *str == '\0'
             ? current_len                              // end of recursion
             : StringLength(str + 1, current_len + 1);  // compute recursively
}

// validates that a character is part of a binary string
constexpr bool IsBinary(char c) {
  return c == '0' || c == '1';
}

// implementation function called after validating string length
template <typename OutputType = unsigned>
constexpr unsigned _BinaryLiteral(const char* str, size_t val = 0) {
  return StringLength(str) == 0
             ? val             // end of recursion
             : IsBinary(*str)  // check for non-binary digit
                   ? _BinaryLiteral(str + 1, 2 * val + *str - '0')
                   : throw std::logic_error("char is not '0' or '1'");
}

}  // namespace detail

template <typename OutputType = unsigned>
constexpr OutputType BinaryLiteral(const char* str, size_t val = 0) {
  return detail::StringLength(str) <= detail::NumberOfBits<OutputType>()
             ? detail::_BinaryLiteral(str, val)
             : throw std::logic_error("Binary literal is too long for type");
}

#define BINARY_LITERAL(X) BinaryLiteral(#X)

#if (__cplusplus >= 201103L)
constexpr unsigned operator"" _b(const char* str) {
  return BinaryLiteral(str);
}
#endif
