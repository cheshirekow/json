#pragma once
// Copyright 2018 Josh Bialkowski <josh.bialkowski@gmail.com>

#include <string>
#include <vector>

#include "json/json.h"

namespace json {
namespace item {

struct Item;

// Either an object or a list
struct Group {
  Item* head_;
  Item* tail_;
  void append(Item* item);
};

struct Object : public Group {};
struct List : public Group {};

union ItemStore {
  Object object;
  List list;

  re2::StringPiece string;
  double floatval;
  int64_t integer;
  bool boolean;

  ItemStore() {
    memset(this, 0, sizeof(ItemStore));
  }
};

struct Item {
  /// Enumerates json value types
  enum TypeNo {
    JSON_INVALID = 0,
    JSON_OBJECT,
    JSON_KEY,
    JSON_LIST,
    JSON_INTEGER,
    JSON_FLOAT,
    JSON_BOOLEAN,
    JSON_NULL,
    JSON_STRING,
  };

  TypeNo typeno;    //< typeid
  ItemStore store;  //< content data
  Item* next;       //< next item in the current list/object

  // TODO(josh): evaluate if there's a performance improvement for inlining
  // these functions.
  void assign_key(const re2::StringPiece& string);
  void operator=(const re2::StringPiece& string);
  void operator=(double inval);
  void operator=(int64_t inval);
  void operator=(bool inval);
  void operator=(std::nullptr_t);
  void operator=(const List& list);
  void operator=(const Object& object);
  Group* as_group();

  // object accessor, assume object is an object and select an item out of it
  // by key
  const Item& operator[](const char* key) const;
  const Item& operator[](const std::string& key) const;

  // List accessor, assume object is a list and select an item out of it.
  const Item& operator[](size_t idx) const;
};

class ItemParser : public Parser {
 public:
  ItemParser(Item* begin, Item* end);
  Item* alloc_item(Error* error);
  int consume(const Token& tok, Error* error);

 private:
  Item* mem_begin_;
  Item* mem_write_;
  Item* mem_end_;

  // TODO(josh): replace with a custom structure with fixed size
  std::vector<Item*> item_stack_;
};

}  // namespace item
}  // namespace json
