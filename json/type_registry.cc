// Copyright 2018 Josh Bialkowski <josh.bialkowski@gmail.com>
#include "json/type_registry.h"

#include <cstdint>
#include <string>

#include "json/util.h"

namespace json {
namespace stream {

template <typename T>
int primitive_helper(const T& value, Dumper* dumper) {
  dumper->dump_primitive(value);
  return 0;
}

int parse_stringpiece(const Token& token, re2::StringPiece* out) {
  LOG(WARNING) << "Attempt to parse into a StringPiece which is const";
  return 1;
}

int parse_string(const Token& token, std::string* out) {
  // NOTE(josh): strip literal quotes from beginning/end of the string.
  re2::StringPiece unquoted =
      token.spelling.substr(1, token.spelling.size() - 2);
  out->resize(unquoted.size());
  size_t nchars = unescape(unquoted, &(*out)[0], &(*out)[0] + unquoted.size());
  out->resize(nchars);
  return 0;
}

// -----------------------------------------------------------------------------
//    class Registry
// -----------------------------------------------------------------------------

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

// -----------------------------------------------------------------------------
//    class Dumper
// -----------------------------------------------------------------------------

Dumper::Dumper(const Registry* registry, const SerializeOpts& opts)
    : registry_{registry}, opts_(opts) {
  if (!registry) {
    registry_ = global_registry();
  }
}

// -----------------------------------------------------------------------------
//    class DumpGuard
// -----------------------------------------------------------------------------

DumpGuard::DumpGuard(Dumper* dumper, GuardType type)
    : dumper_(dumper), type_(type) {
  switch (type_) {
    case GUARD_OBJECT:
      dumper_->dump_event(DumpEvent::OBJECT_BEGIN);
      break;
    case GUARD_LIST:
      dumper_->dump_event(DumpEvent::LIST_BEGIN);
      break;
  }
}

DumpGuard::~DumpGuard() {
  switch (type_) {
    case GUARD_OBJECT:
      dumper_->dump_event(DumpEvent::OBJECT_END);
      break;
    case GUARD_LIST:
      dumper_->dump_event(DumpEvent::LIST_END);
      break;
  }
}

// -----------------------------------------------------------------------------
//    class StreamDumper
// -----------------------------------------------------------------------------

StreamDumper::StreamDumper(std::ostream* ostream, const SerializeOpts& opts,
                           const Registry* registry)
    : Dumper(registry, opts), ostream_{ostream} {}

void StreamDumper::dump_event(DumpEvent::TypeNo eventno) {
  switch (eventno) {
    case DumpEvent::LIST_BEGIN:
      (*ostream_) << "[";
      if (opts_.indent) {
        (*ostream_) << "\n";
      }
      dump_stack_.push_back({DumpStack::LIST, 0});
      break;

    case DumpEvent::LIST_END:
      assert(dump_stack_.size());
      assert(dump_stack_.back().type == DumpStack::LIST);
      if (opts_.indent && dump_stack_.back().count) {
        (*ostream_) << "\n";
        for (size_t idx = 0; idx < (dump_stack_.size() - 1) * opts_.indent;
             idx++) {
          (*ostream_) << ' ';
        }
      }
      (*ostream_) << "]";
      dump_stack_.pop_back();
      break;

    case DumpEvent::OBJECT_BEGIN:
      (*ostream_) << "{";
      if (opts_.indent) {
        (*ostream_) << "\n";
      }
      dump_stack_.push_back({DumpStack::OBJECT, 0});
      break;

    case DumpEvent::OBJECT_END:
      assert(dump_stack_.size());
      assert(dump_stack_.back().type == DumpStack::OBJECT);
      if (opts_.indent && dump_stack_.back().count) {
        (*ostream_) << "\n";
        for (size_t idx = 0; idx < (dump_stack_.size() - 1) * opts_.indent;
             idx++) {
          (*ostream_) << ' ';
        }
      }
      (*ostream_) << "}";
      dump_stack_.pop_back();
      break;

    case DumpEvent::LIST_VALUE:
    case DumpEvent::OBJECT_KEY:
      // This is the start of a field in an object or a value in a list,
      // if it is not the first, then write out the "," separator
      if (dump_stack_.size() && dump_stack_.back().count) {
        (*ostream_) << opts_.separators[1];
        if (opts_.indent) {
          (*ostream_) << "\n";
        }
      }
      for (size_t idx = 0; idx < dump_stack_.size() * opts_.indent; idx++) {
        (*ostream_) << ' ';
      }

      // This is *not* the end of an aggregate, so whatever aggregate is at
      // the top of the stack is getting a new element
      dump_stack_.back().count += 1;
      break;

    case DumpEvent::OBJECT_VALUE:
      // This is the value for a field, following the field name, so we
      // should write out the ":" separator.
      (*ostream_) << opts_.separators[0];
      break;
    default:
      break;
  }
}

void StreamDumper::dump_primitive(uint8_t value) {
  (*ostream_) << static_cast<int32_t>(value);
}

void StreamDumper::dump_primitive(uint16_t value) {
  (*ostream_) << value;
}

void StreamDumper::dump_primitive(uint32_t value) {
  (*ostream_) << value;
}

void StreamDumper::dump_primitive(uint64_t value) {
  (*ostream_) << value;
}

void StreamDumper::dump_primitive(int8_t value) {
  (*ostream_) << static_cast<int32_t>(value);
}

void StreamDumper::dump_primitive(int16_t value) {
  (*ostream_) << value;
}

void StreamDumper::dump_primitive(int32_t value) {
  (*ostream_) << value;
}

void StreamDumper::dump_primitive(int64_t value) {
  (*ostream_) << value;
}

void StreamDumper::dump_primtiive(float value) {
  (*ostream_) << value;
}

void StreamDumper::dump_primitive(double value) {
  (*ostream_) << value;
}

void StreamDumper::dump_primitive(bool value) {
  (*ostream_) << (value ? "true" : "false");
}

void StreamDumper::dump_primitive(std::nullptr_t nullval) {
  (*ostream_) << "null";
}

void StreamDumper::dump_primitive(re2::StringPiece strval) {
  (*ostream_) << '\"';
  escape(strval, ostream_);
  (*ostream_) << '\"';
}

void StreamDumper::dump_primitive(const std::string& strval) {
  (*ostream_) << '\"';
  escape(strval, ostream_);
  (*ostream_) << '\"';
}

void StreamDumper::dump_primitive(const char* strval) {
  (*ostream_) << '\"';
  escape(strval, ostream_);
  (*ostream_) << '\"';
}

}  // namespace stream
}  // namespace json
