// Copyright 2018 Josh Bialkowski <josh.bialkowski@gmail.com>
#include "tangent/json/variant.h"
#include "tangent/util/fixed_string_stream.h"

namespace json {
namespace variant {

NullType Null;

Variant::Variant() : typeno(INVALID) {
  memset(&store, 0, sizeof(Store));
}

Variant::Variant(TypeNo typeno) : typeno(typeno) {
  switch (typeno) {
    case OBJECT:
      new (&store.object) Object();
      return;
    case LIST:
      new (&store.list) List();
      return;
    case STRING:
      new (&store.string) String();
    default:
      memset(&store, 0, sizeof(Store));
  }
}

Variant::Variant(const Variant& other) : typeno(INVALID) {
  assign(other);
}

Variant::Variant(const List& value) : typeno(INVALID) {
  assign(value);
}

Variant::Variant(const Object& other) : typeno(INVALID) {
  assign(other);
}

Variant::Variant(const String& value) : typeno(INVALID) {
  assign(value);
}

Variant::Variant(int64_t value) : typeno(INVALID) {
  assign(value);
}

Variant::Variant(double value) : typeno(INVALID) {
  assign(value);
}

Variant::Variant(bool value) : typeno(INVALID) {
  assign(value);
}

Variant::Variant(NullType value) : typeno(INVALID) {
  assign(value);
}

Variant::~Variant() {
  clear();
}

Variant& Variant::operator=(const Variant& other) {
  assign(other);
  return *this;
}
Variant& Variant::operator=(const List& other) {
  assign(other);
  return *this;
}
Variant& Variant::operator=(const Object& other) {
  assign(other);
  return *this;
}
Variant& Variant::operator=(const String& value) {
  assign(value);
  return *this;
}
Variant& Variant::operator=(const int64_t value) {
  assign(value);
  return *this;
}
Variant& Variant::operator=(const double value) {
  assign(value);
  return *this;
}
Variant& Variant::operator=(const bool value) {
  assign(value);
  return *this;
}
Variant& Variant::operator=(const NullType value) {
  assign(value);
  return *this;
}

static Variant kInvalid;

const Variant& Variant::operator[](const std::string& key) const {
  return this->get(key);
}

Variant& Variant::operator[](const std::string& key) {
  return this->get(key);
}

const Variant& Variant::operator[](const char* key) const {
  return this->get(key);
}

Variant& Variant::operator[](const char* key) {
  return this->get(key);
}

Variant& Variant::operator[](size_t idx) {
  return this->at(idx);
}

const Variant& Variant::operator[](size_t idx) const {
  return this->at(idx);
}

Variant::operator List() {
  assert(typeno == LIST);
  return store.list;
}

Variant::operator Object() {
  assert(typeno == OBJECT);
  return store.object;
}

Variant::operator String() {
  assert(typeno == STRING);
  return store.string;
}

Variant::operator int64_t() {
  assert(typeno == INTEGER);
  return store.integer;
}

Variant::operator double() {
  assert(typeno == REALNO);
  return store.realno;
}

Variant::operator bool() {
  assert(typeno == BOOLEAN);
  return store.boolean;
}

Variant::operator NullType() {
  assert(typeno == JNULL);
  return Null;
}

void Variant::assign(const Variant& other) {
  clear();
  typeno = other.typeno;
  switch (other.typeno) {
    case OBJECT:
      new (&store.object) Object(other.store.object);
      return;
    case LIST:
      new (&store.list) List(other.store.list);
      return;
    case STRING:
      new (&store.string) String(other.store.string);
      return;
    default:
      memcpy(this, &other, sizeof(Variant));
  }
}

void Variant::assign(const List& value) {
  clear();
  typeno = LIST;
  new (&store.list) List(value);
}

void Variant::assign(const Object& value) {
  clear();
  typeno = OBJECT;
  new (&store.object) Object(value);
}

void Variant::assign(const String& value) {
  clear();
  typeno = STRING;
  new (&store.string) String(value);
}

void Variant::assign(int64_t value) {
  clear();
  typeno = INTEGER;
  store.integer = value;
}

void Variant::assign(double value) {
  clear();
  typeno = REALNO;
  store.realno = value;
}

void Variant::assign(bool value) {
  clear();
  typeno = BOOLEAN;
  store.boolean = value;
}

void Variant::assign(NullType value) {
  clear();
  typeno = JNULL;
}

void Variant::clear() {
  switch (typeno) {
    case LIST:
      store.list.~List();
      break;
    case OBJECT:
      store.object.~Object();
      break;
    case STRING:
      store.string.~String();
      break;
    default:
      break;
  }
  typeno = INVALID;
}

const Variant& Variant::get(const std::string& key) const {
  return this->get(key.c_str());
}

Variant& Variant::get(const std::string& key) {
  return this->get(key.c_str());
}

const Variant& Variant::get(const char* key) const {
  assert(typeno == OBJECT);
  if (typeno != OBJECT) {
    return kInvalid;
  }
  auto iter = store.object.find(key);
  assert(iter != store.object.end());
  if (iter == store.object.end()) {
    return kInvalid;
  }
  return iter->second;
}

Variant& Variant::get(const char* key) {
  assert(typeno == OBJECT);
  if (typeno != OBJECT) {
    // TODO(josh): dont' return modifiable invalid
    return kInvalid;
  }
  return store.object[key];
}

Variant& Variant::at(size_t idx) {
  assert(typeno == LIST);
  if (typeno != LIST) {
    // TODO(josh): dont' return modifiable invalid
    return kInvalid;
  }
  if (idx < store.list.size()) {
    store.list.resize(idx + 1, kInvalid);
  }
  return store.list[idx];
}

const Variant& Variant::at(size_t idx) const {
  assert(typeno == LIST);
  if (typeno != LIST) {
    return kInvalid;
  }
  if (idx < store.list.size()) {
    return kInvalid;
  }
  return store.list[idx];
}

size_t Variant::serialize(char* begin, char* end,
                          const SerializeOpts& opts) const {
  util::FixedBufStream<char> printer{begin, end};
  this->serialize(&printer, opts, 0);
  return printer.size();
}

void fmt_indent(std::ostream* out, size_t indent, size_t depth) {
  for (size_t idx = 0; idx < indent * depth; idx++) {
    (*out) << ' ';
  }
}

void Variant::serialize(std::ostream* out, const SerializeOpts& opts,
                        size_t depth) const {
  switch (typeno) {
    case LIST: {
      if (store.list.size() < 1) {
        (*out) << "[]";
      } else {
        (*out) << "[";
        auto iter = store.list.begin();
        while (iter != store.list.end()) {
          fmt_indent(out, opts.indent, depth + 1);
          iter->serialize(out, opts, depth + 1);
          ++iter;
          if (iter != store.list.end()) {
            (*out) << opts.separators[1];
          }
          if (opts.indent) {
            (*out) << "\n";
          }
        }
        (*out) << "]";
      }
      break;
    }
    case OBJECT: {
      if (store.object.size() < 1) {
        (*out) << "{}";
      } else {
        (*out) << "{";
        if (opts.indent) {
          (*out) << "\n";
        }
        auto iter = store.object.begin();
        while (iter != store.object.end()) {
          fmt_indent(out, opts.indent, depth + 1);
          (*out) << '"' << iter->first.c_str() << '"' << opts.separators[0];
          iter->second.serialize(out, opts, depth + 1);
          ++iter;
          if (iter != store.object.end()) {
            (*out) << opts.separators[1];
          }
          if (opts.indent) {
            (*out) << "\n";
          }
        }
        fmt_indent(out, opts.indent, depth);
        (*out) << "}";
      }
      break;
    }
    case STRING: {
      (*out) << '"' << store.string.c_str() << '"';
      break;
    }
    case REALNO: {
      (*out) << store.realno;
      break;
    }
    case INTEGER: {
      (*out) << store.integer;
      break;
    }
    case BOOLEAN: {
      (*out) << (store.boolean ? "true" : "false");
      break;
    }
    case JNULL: {
      (*out) << "null";
      break;
    }
    default:
      break;
  }
}

}  // namespace variant
}  // namespace json
