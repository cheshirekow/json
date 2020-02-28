// Copyright 2019 Josh Bialkowski <josh.bialkowski@gmail.com>
#include "json/util.h"

#include <map>
#include <sstream>

#include <fmt/format.h>
#include <fmt/ostream.h>
#include <re2/stringpiece.h>

namespace json {

template <class T, class U>
std::map<U, T> invert_map(const std::map<T, U>& inmap) {
  std::map<U, T> outmap;
  for (auto pair : inmap) {
    outmap.insert(std::make_pair(pair.second, pair.first));
  }
  return outmap;
}

/// Return true if the given character is a control code
bool is_control_code(char code) {
  return ('\x00' <= code && code <= '\x1f');
}

/// Map control codes that have a JSON shortcode to their shortcode
const std::map<char, char> kEscapeMap = {
    // clang-format off
    {'"', '"'},
    {'\\', '\\'},
    {'\b', 'b'},
    {'\f', 'f'},
    {'\n', 'n'},
    {'\r', 'r'},
    {'\t', 't'},
    // clang-format on
};

// adapted from: https://stackoverflow.com/a/33799784/141023
void escape(const re2::StringPiece& piece, std::ostream* out) {
  for (const char* ptr = piece.begin(); ptr != piece.end(); ptr++) {
    auto iter = kEscapeMap.find(*ptr);
    if (iter != kEscapeMap.end()) {
      (*out) << '\\';
      (*out) << iter->second;
    } else if (is_control_code(*ptr)) {
      // (*out) << "\\u" << std::hex << std::setw(4) << std::setfill('0')
      //        << (int)*ptr;
      fmt::print(*out, "\\u{:04x}", static_cast<int>(*ptr));
    } else {
      (*out) << *ptr;
    }
  }
}

std::string escape(const re2::StringPiece& piece) {
  std::stringstream strm;
  escape(piece, &strm);
  return strm.str();
}

/// Map control shortcodes to the controlcode
const std::map<char, char> kUnescapeMap = invert_map(kEscapeMap);

void unescape(const re2::StringPiece& piece, std::ostream* out) {
  for (const char* ptr = piece.begin(); ptr != piece.end(); ++ptr) {
    if (ptr + 1 == piece.end()) {
      (*out) << *ptr;
      continue;
    }
    if (ptr[0] == '\\') {
      auto iter = kUnescapeMap.find(ptr[1]);
      if (iter != kUnescapeMap.end()) {
        (*out) << iter->second;
        ptr++;
      } else if (ptr[1] == 'u' && ptr + 5 < piece.end()) {
        std::string value = piece.substr(2, 4).as_string();
        int x = std::stoi(value, nullptr, 16);
        (*out) << static_cast<char>(x);
        ptr += 5;
      } else {
        (*out) << *ptr;
      }
    } else {
      (*out) << *ptr;
    }
  }
}

std::string unescape(const re2::StringPiece& piece) {
  std::stringstream strm;
  unescape(piece, &strm);
  return strm.str();
}

void unescape(const re2::StringPiece& piece, char* begin, char* end) {
  char* out = begin;
  for (const char* ptr = piece.begin(); ptr != piece.end() && out != end;
       ++ptr) {
    if (ptr + 1 == piece.end()) {
      (*out++) = *ptr;
      continue;
    }
    if (ptr[0] == '\\') {
      auto iter = kUnescapeMap.find(ptr[1]);
      if (iter != kUnescapeMap.end()) {
        (*out++) = iter->second;
        ptr++;
      } else if (ptr[1] == 'u' && ptr + 5 < piece.end()) {
        std::string value = piece.substr(2, 4).as_string();
        int x = std::stoi(value, nullptr, 16);
        (*out++) = static_cast<char>(x);
        ptr += 5;
      } else {
        (*out++) = *ptr;
      }
    } else {
      (*out++) = *ptr;
    }
  }

  if (out != end) {
    (*out) = '\0';
  } else if (begin != end) {
    *(end - 1) = '\0';
  }
}

}  // namespace json
