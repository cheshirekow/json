#pragma once
// Copyright 2018 Josh Bialkowski <josh.bialkowski@gmail.com>

#include <sstream>
#include <string>
#include <vector>

namespace string {

const std::string or_none(const char* value);

std::string to_lower(const std::string& instr);

std::vector<std::string> split(const std::string& str, char delim = ' ');

bool starts_with(const std::string& haystack, const std::string& needle,
                 bool case_sensitive = true);

template <class Container>
inline std::string join(const Container& elems, const std::string& glue = ",");

template <typename Out>
void split(const std::string& s, char delim, Out result);

}  // namespace string

// -----------------------------------------------------------------------------
//                          Template Impls
// -----------------------------------------------------------------------------
namespace string {

template <class Container>
inline std::string join(const Container& elems, const std::string& glue) {
  std::stringstream strm;
  auto iter = elems.begin();
  if (iter != elems.end()) {
    strm << *iter;
  }
  for (++iter; iter != elems.end(); ++iter) {
    strm << glue;
    strm << *iter;
  }
  return strm.str();
}

template <typename Out>
void split(const std::string& s, char delim, Out result) {
  std::stringstream ss(s);
  std::string item;
  while (std::getline(ss, item, delim)) {
    *(result++) = item;
  }
}

}  // namespace string
