// Copyright 2018 Josh Bialkowski <josh.bialkowski@gmail.com>
#include "util/string_util.h"

#include <algorithm>
#include <iterator>

namespace string {

const std::string or_none(const char* value) {
  return value ? value : "<None>";
}

std::vector<std::string> split(const std::string& s, char delim) {
  std::vector<std::string> elems;
  split(s, delim, std::back_inserter(elems));
  return elems;
}

std::string to_lower(const std::string& instr) {
  std::string outstr(instr.size(), '0');
  std::transform(instr.begin(), instr.end(), outstr.begin(), ::tolower);
  return outstr;
}

bool starts_with(const std::string& haystack, const std::string& needle,
                 bool case_sensitive) {
  if (case_sensitive) {
    return haystack.substr(0, needle.size()) == needle;
  } else {
    return to_lower(haystack.substr(0, needle.size())) == to_lower(needle);
  }
}

}  // namespace string
