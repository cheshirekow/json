#pragma once
// Copyright 2018 Josh Bialkowski <josh.bialkowski@gmail.com>

#include <sstream>
#include <string>
#include <vector>

namespace string {

// Return a std::string from a c-string, or the value "None" if the pointer
// is empty.
const std::string or_none(const char* value);

// Return a lowercase copy of the string
std::string to_lower(const std::string& value);

// Return an uppercase copy of the string
std::string to_upper(const std::string& value);

// Split a string at each occurance of a delimeter
std::vector<std::string> split(const std::string& str, char delim = ' ');

// Return true if `needle` is a prefix of `haystack'.
bool starts_with(const std::string& haystack, const std::string& needle,
                 bool case_sensitive = true);

// Create a string by joining the elements of the container using default
// stream formatting and the provided delimeter. Uses
// ``ostream& operator<<(ostream&, ...)`` to format each element
template <class Container>
inline std::string join(const Container& elems, const std::string& glue = ", ");

// Split a string at each occurance of a delimieter. Push each item to the
// output container.
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
