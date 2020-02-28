// Copyright (C) 2012 Josh Bialkowski (josh.bialkowski@gmail.com)
/**
 *  @file
 *  @date   Aug 4, 2012
 *  @author Josh Bialkowski (josh.bialkowski@gmail.com)
 */
#pragma once

namespace detail {

template <typename T>
struct RangeImpl {
  struct Iterator {
    T val;  ///< storage for the actual value

    explicit Iterator(T val) : val(val) {}

    T operator*() {
      return val;
    }

    bool operator!=(T other) {
      return val != other;
    }

    Iterator& operator++() {
      ++val;
      return *this;
    }

    operator T() {
      return val;
    }
  };

 private:
  T m_begin;  ///< the first integral value
  T m_end;    ///< one past the last integral value

 public:
  RangeImpl(T begin, T end) : m_begin(begin), m_end(end) {}

  T size() {
    return m_end - m_begin;
  }

  Iterator begin() {
    return m_begin;
  }

  Iterator end() {
    return m_end;
  }
};

}  // namespace detail

template <typename T>
detail::RangeImpl<T> Range(T begin, T end) {
  return detail::RangeImpl<T>(begin, end);
}
