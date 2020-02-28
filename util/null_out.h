#pragma once
// Copyright (C) 2018 Josh Bialkowski <josh.bialkowski@gmail.com>

#include <iterator>

/// dummy output iterator discards assignment
struct NullOut : std::iterator<std::output_iterator_tag, NullOut> {
  template <typename T>
  void operator=(T const&) {}

  NullOut& operator++() {
    return *this;
  }

  NullOut operator++(int) {
    return *this;
  }

  NullOut& operator*() {
    return *this;
  }
};
