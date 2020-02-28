// Copyright 2018 Josh Bialkowski <josh.bialkowski@gmail.com>
#include "json/type_registry.h"
#include <cstdint>

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
}

Registry* global_registry() {
  static Registry* g_registry_ = new Registry{};
  return g_registry_;
}

}  // namespace stream
}  // namespace json
