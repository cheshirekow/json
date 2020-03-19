#pragma once

// Copyright 2018 Josh Bialkowski <josh.bialkowski@gmail.com>
#include "tangent/json/variant.h"

namespace json {
namespace builder {

struct ListBuilder;

struct ObjectBuilder {
  void consume(const char* key, const char* value);
  void consume(const char* key, const int value);
  void consume(const char* key, const double value);
  void consume(const char* key, const bool value);
  void consume(const char* key, const std::nullptr_t);
  void consume(const char* key, const ObjectBuilder& other);
  void consume(const char* key, const ListBuilder& other);

  // TODO(josh): try to init with a Pair<T,U> and see if that works, would
  // make things a little more legible when writing them out.
  template <typename Head_>
  void init_with(const char* key, const Head_& head) {
    consume(key, head);
  }

  template <typename Head_, typename... Tail_>
  void init_with(const char* key, const Head_& head, Tail_... tail) {
    consume(key, head);
    init_with(tail...);
  }

  template <typename... Tail_>
  ObjectBuilder(const char* key, Tail_... tail) : var_(variant::OBJECT) {
    init_with(key, tail...);
  }

  ObjectBuilder() : var_(variant::OBJECT) {}
  ObjectBuilder(const ObjectBuilder& other) : var_(other.var_) {}
  ~ObjectBuilder() {}

  Variant var_;
};

struct ListBuilder {
  void consume(const char* value);
  void consume(const int value);
  void consume(const double value);
  void consume(const bool value);
  void consume(const std::nullptr_t);
  void consume(const ObjectBuilder& other);
  void consume(const ListBuilder& other);

  template <typename Head_>
  void init_with(const Head_& head) {
    consume(head);
  }

  template <typename Head_, typename... Tail_>
  void init_with(const Head_& head, Tail_... tail) {
    consume(head);
    init_with(tail...);
  }

  template <typename Head_>
  ListBuilder(const Head_& head)  // NOLINT(runtime/explicit)
      : var_(variant::LIST) {}

  template <typename... Tail_>
  ListBuilder(const Tail_&... tail)  // NOLINT(runtime/explicit)
      : var_(variant::LIST) {
    init_with(tail...);
  }

  ListBuilder() : var_(variant::LIST) {}
  ListBuilder(const ListBuilder& other) : var_(other.var_) {}
  ~ListBuilder() {}

  Variant var_;
};

}  // namespace builder

namespace insource {
typedef builder::ObjectBuilder O;
typedef builder::ListBuilder L;
}  // namespace insource

Variant build(const builder::ObjectBuilder& o);
Variant build(const builder::ListBuilder& o);

}  // namespace json
