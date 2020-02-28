// Copyright 2018 Josh Bialkowski <josh.bialkowski@gmail.com>
#include "json/type_registry.h"

#include <cstdint>
#include <string>

#include "json/util.h"

namespace json {
namespace stream {

template <typename T>
int primitive_helper(Dumper* dumper, const T& value) {
  dumper->dump_primitive(value);
  return 0;
}

void parse_stringpiece(const Token& token, re2::StringPiece* out) {
  LOG(WARNING) << "Attempt to parse into a StringPiece which is const";
}

void parse_string(const Token& token, std::string* out) {
  // NOTE(josh): strip literal quotes from beginning/end of the string.
  re2::StringPiece unquoted =
      token.spelling.substr(1, token.spelling.size() - 2);
  out->resize(unquoted.size());
  size_t nchars = unescape(unquoted, &(*out)[0], &(*out)[0] + unquoted.size());
  out->resize(nchars);
}

Registry::Registry() {
  register_scalar(ParseInteger<uint8_t>, primitive_helper<uint8_t>);
  register_scalar(ParseInteger<uint16_t>, primitive_helper<uint16_t>);
  register_scalar(ParseInteger<uint32_t>, primitive_helper<uint32_t>);
  register_scalar(ParseInteger<uint64_t>, primitive_helper<uint64_t>);
  register_scalar(ParseInteger<int8_t>, primitive_helper<int8_t>);
  register_scalar(ParseInteger<int16_t>, primitive_helper<int16_t>);
  register_scalar(ParseInteger<int32_t>, primitive_helper<int32_t>);
  register_scalar(ParseInteger<int64_t>, primitive_helper<int64_t>);
  register_scalar(ParseRealNumber<double>, primitive_helper<double>);
  register_scalar(ParseRealNumber<float>, primitive_helper<float>);
  register_scalar(ParseBoolean, primitive_helper<bool>);
  register_scalar(parse_stringpiece, primitive_helper<re2::StringPiece>);
  register_scalar(parse_string, primitive_helper<std::string>);
}

Registry* global_registry() {
  static Registry* g_registry_ = new Registry{};
  return g_registry_;
}

}  // namespace stream
}  // namespace json
