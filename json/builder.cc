// Copyright 2018 Josh Bialkowski <josh.bialkowski@gmail.com>
#include "json/builder.h"

namespace json {
namespace builder {

void ObjectBuilder::consume(const char* key, const char* value) {
  if (value) {
    var_.store.object[key] = variant::String(value);
  } else {
    var_.store.object[key] = variant::Null;
  }
}

void ObjectBuilder::consume(const char* key, const int value) {
  var_.store.object[key] = int64_t(value);
}
void ObjectBuilder::consume(const char* key, const double value) {
  var_.store.object[key] = value;
}
void ObjectBuilder::consume(const char* key, const bool value) {
  var_.store.object[key] = value;
}

void ObjectBuilder::consume(const char* key, const std::nullptr_t) {
  var_.store.object[key] = variant::Null;
}

void ObjectBuilder::consume(const char* key, const ObjectBuilder& other) {
  var_.store.object[key] = other.var_;
}

void ObjectBuilder::consume(const char* key, const ListBuilder& other) {
  var_.store.object[key] = other.var_;
}

void ListBuilder::consume(const char* value) {
  if (value) {
    var_.store.list.emplace_back(variant::String(value));
  } else {
    var_.store.list.emplace_back(variant::Null);
  }
}
void ListBuilder::consume(const int value) {
  var_.store.list.emplace_back(int64_t(value));
}
void ListBuilder::consume(const double value) {
  var_.store.list.emplace_back(value);
}
void ListBuilder::consume(const bool value) {
  var_.store.list.emplace_back(value);
}

void ListBuilder::consume(const std::nullptr_t) {
  var_.store.list.emplace_back(variant::Null);
}

void ListBuilder::consume(const ObjectBuilder& other) {
  var_.store.list.emplace_back(other.var_);
}

void ListBuilder::consume(const ListBuilder& other) {
  var_.store.list.emplace_back(other.var_);
}

}  // namespace builder

Variant build(const builder::ObjectBuilder& o) {
  return Variant(o.var_);
}

Variant build(const builder::ListBuilder& o) {
  return Variant(o.var_);
}

}  // namespace json
