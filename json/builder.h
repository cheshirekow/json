#pragma once

// Copyright 2018 Josh Bialkowski <josh.bialkowski@gmail.com>
#include "json/variant.h"

namespace json {
namespace builder {

struct ListBuilder;

struct ObjectBuilder {
  void Consume(const char* key, const char* value);
  void Consume(const char* key, const int value);
  void Consume(const char* key, const double value);
  void Consume(const char* key, const bool value);
  void Consume(const char* key, const std::nullptr_t);
  void Consume(const char* key, const ObjectBuilder& other);
  void Consume(const char* key, const ListBuilder& other);

  // TODO(josh): try to init with a Pair<T,U> and see if that works, would
  // make things a little more legible when writing them out.
  template <typename Head_>
  void InitWith(const char* key, const Head_& head) {
    Consume(key, head);
  }

  template <typename Head_, typename... Tail_>
  void InitWith(const char* key, const Head_& head, Tail_... tail) {
    Consume(key, head);
    InitWith(tail...);
  }

  template <typename... Tail_>
  ObjectBuilder(const char* key, Tail_... tail) : var_(variant::OBJECT) {
    InitWith(key, tail...);
  }

  ObjectBuilder() : var_(variant::OBJECT) {}
  ObjectBuilder(const ObjectBuilder& other) : var_(other.var_) {}
  ~ObjectBuilder() {}

  Variant var_;
};

struct ListBuilder {
  void Consume(const char* value);
  void Consume(const int value);
  void Consume(const double value);
  void Consume(const bool value);
  void Consume(const std::nullptr_t);
  void Consume(const ObjectBuilder& other);
  void Consume(const ListBuilder& other);

  template <typename Head_>
  void InitWith(const Head_& head) {
    Consume(head);
  }

  template <typename Head_, typename... Tail_>
  void InitWith(const Head_& head, Tail_... tail) {
    Consume(head);
    InitWith(tail...);
  }

  template <typename Head_>
  ListBuilder(const Head_& head)  // NOLINT(runtime/explicit)
      : var_(variant::LIST) {}

  template <typename... Tail_>
  ListBuilder(const Tail_&... tail)  // NOLINT(runtime/explicit)
      : var_(variant::LIST) {
    InitWith(tail...);
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

Variant Build(const builder::ObjectBuilder& o);
Variant Build(const builder::ListBuilder& o);

}  // namespace json
