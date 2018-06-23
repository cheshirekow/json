// Copyright 2018 Josh Bialkowski <josh.bialkowski@gmail.com>
#include "json/builder.h"

namespace json {
namespace builder {

void ObjectBuilder::Consume(const char* key, const char* value) {
  if (value) {
    var_.store.object[key] = variant::String(value);
  } else {
    var_.store.object[key] = variant::Null;
  }
}

void ObjectBuilder::Consume(const char* key, const int value) {
  var_.store.object[key] = int64_t(value);
}
void ObjectBuilder::Consume(const char* key, const double value) {
  var_.store.object[key] = value;
}
void ObjectBuilder::Consume(const char* key, const bool value) {
  var_.store.object[key] = value;
}

void ObjectBuilder::Consume(const char* key, const std::nullptr_t) {
  var_.store.object[key] = variant::Null;
}

void ObjectBuilder::Consume(const char* key, const ObjectBuilder& other) {
  var_.store.object[key] = other.var_;
}

void ObjectBuilder::Consume(const char* key, const ListBuilder& other) {
  var_.store.object[key] = other.var_;
}

void ListBuilder::Consume(const char* value) {
  if (value) {
    var_.store.list.emplace_back(variant::String(value));
  } else {
    var_.store.list.emplace_back(variant::Null);
  }
}
void ListBuilder::Consume(const int value) {
  var_.store.list.emplace_back(int64_t(value));
}
void ListBuilder::Consume(const double value) {
  var_.store.list.emplace_back(value);
}
void ListBuilder::Consume(const bool value) {
  var_.store.list.emplace_back(value);
}

void ListBuilder::Consume(const std::nullptr_t) {
  var_.store.list.emplace_back(variant::Null);
}

void ListBuilder::Consume(const ObjectBuilder& other) {
  var_.store.list.emplace_back(other.var_);
}

void ListBuilder::Consume(const ListBuilder& other) {
  var_.store.list.emplace_back(other.var_);
}

}  // namespace builder

Variant Build(const builder::ObjectBuilder& o) {
  return Variant(o.var_);
}

Variant Build(const builder::ListBuilder& o) {
  return Variant(o.var_);
}

}  // namespace json
