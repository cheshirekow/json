#pragma once
// Copyright 2018 Josh Bialkowski <josh.bialkowski@gmail.com>

#include <cassert>
#include <list>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include <re2/re2.h>

#include "json/json.h"

namespace json {
namespace variant {

struct Variant;

typedef std::vector<Variant> List;
typedef std::map<std::string, Variant> Object;
typedef std::basic_string<char> String;
typedef std::nullptr_t NullType;

enum TypeNo {
  INVALID,  //
  OBJECT,
  LIST,
  STRING,
  REALNO,
  INTEGER,
  BOOLEAN,
  JNULL
};

// Provides storage for the different underlying variant types.
union Store {
  Object object;
  List list;
  String string;
  double realno;
  int64_t integer;
  bool boolean;

  Store() {
    memset(this, 0, sizeof(Store));
  }

  ~Store() {}
};

extern NullType Null;

// An object which may be any of the six JSON primitives.
struct Variant {
  TypeNo typeno;
  Store store;

  Variant();
  Variant(const Variant& other);

  explicit Variant(TypeNo typeno);
  explicit Variant(const List& value);
  explicit Variant(const Object& other);
  explicit Variant(const String& value);
  explicit Variant(int64_t value);
  explicit Variant(double value);
  explicit Variant(bool value);
  explicit Variant(NullType value);
  ~Variant();

  Variant& operator=(const Variant& other);
  Variant& operator=(const List& other);
  Variant& operator=(const Object& other);
  Variant& operator=(const String& value);
  Variant& operator=(const int64_t value);
  Variant& operator=(const double value);
  Variant& operator=(const bool value);
  Variant& operator=(const NullType value);

  const Variant& operator[](const char* key) const;
  Variant& operator[](const char* key);
  const Variant& operator[](const std::string& key) const;
  Variant& operator[](const std::string& key);
  const Variant& operator[](size_t idx) const;
  Variant& operator[](size_t idx);

  operator List();
  operator Object();
  operator String();
  operator int64_t();
  operator double();
  operator bool();
  operator NullType();

  void Assign(const Variant& other);
  void Assign(const List& value);
  void Assign(const Object& value);
  void Assign(const String& value);
  void Assign(int64_t value);
  void Assign(double value);
  void Assign(bool value);
  void Assign(NullType value);
  void Clear();

  const Variant& Get(const char* key) const;
  Variant& Get(const char* key);
  const Variant& Get(const std::string& key) const;
  Variant& Get(const std::string& key);
  const Variant& At(size_t idx) const;
  Variant& At(size_t idx);

  size_t Serialize(char* begin, char* end,
                   const SerializeOpts& opts = kDefaultOpts) const;

 private:
  void Serialize(std::ostream* write, const SerializeOpts& opts,
                 size_t depth) const;
};

}  // namespace variant

typedef variant::Variant Variant;

}  // namespace json
