// Copyright 2018 Josh Bialkowski <josh.bialkowski@gmail.com>
#include "json/variant.h"

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
  Assign(other);
}

Variant::Variant(const List& value) : typeno(INVALID) {
  Assign(value);
}

Variant::Variant(const Object& other) : typeno(INVALID) {
  Assign(other);
}

Variant::Variant(const String& value) : typeno(INVALID) {
  Assign(value);
}

Variant::Variant(int64_t value) : typeno(INVALID) {
  Assign(value);
}

Variant::Variant(double value) : typeno(INVALID) {
  Assign(value);
}

Variant::Variant(bool value) : typeno(INVALID) {
  Assign(value);
}

Variant::Variant(NullType value) : typeno(INVALID) {
  Assign(value);
}

Variant::~Variant() {
  Clear();
}

Variant& Variant::operator=(const Variant& other) {
  Assign(other);
  return *this;
}
Variant& Variant::operator=(const List& other) {
  Assign(other);
  return *this;
}
Variant& Variant::operator=(const Object& other) {
  Assign(other);
  return *this;
}
Variant& Variant::operator=(const String& value) {
  Assign(value);
  return *this;
}
Variant& Variant::operator=(const int64_t value) {
  Assign(value);
  return *this;
}
Variant& Variant::operator=(const double value) {
  Assign(value);
  return *this;
}
Variant& Variant::operator=(const bool value) {
  Assign(value);
  return *this;
}
Variant& Variant::operator=(const NullType value) {
  Assign(value);
  return *this;
}

static Variant kInvalid;

const Variant& Variant::operator[](const std::string& key) const {
  return this->Get(key);
}

Variant& Variant::operator[](const std::string& key) {
  return this->Get(key);
}

const Variant& Variant::operator[](const char* key) const {
  return this->Get(key);
}

Variant& Variant::operator[](const char* key) {
  return this->Get(key);
}

Variant& Variant::operator[](size_t idx) {
  return this->At(idx);
}

const Variant& Variant::operator[](size_t idx) const {
  return this->At(idx);
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

void Variant::Assign(const Variant& other) {
  Clear();
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

void Variant::Assign(const List& value) {
  Clear();
  typeno = LIST;
  new (&store.list) List(value);
}

void Variant::Assign(const Object& value) {
  Clear();
  typeno = OBJECT;
  new (&store.object) Object(value);
}

void Variant::Assign(const String& value) {
  Clear();
  typeno = STRING;
  new (&store.string) String(value);
}

void Variant::Assign(int64_t value) {
  Clear();
  typeno = INTEGER;
  store.integer = value;
}

void Variant::Assign(double value) {
  Clear();
  typeno = REALNO;
  store.realno = value;
}

void Variant::Assign(bool value) {
  Clear();
  typeno = BOOLEAN;
  store.boolean = value;
}

void Variant::Assign(NullType value) {
  Clear();
  typeno = JNULL;
}

void Variant::Clear() {
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

const Variant& Variant::Get(const std::string& key) const {
  return this->Get(key.c_str());
}

Variant& Variant::Get(const std::string& key) {
  return this->Get(key.c_str());
}

const Variant& Variant::Get(const char* key) const {
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

Variant& Variant::Get(const char* key) {
  assert(typeno == OBJECT);
  if (typeno != OBJECT) {
    // TODO(josh): dont' return modifiable invalid
    return kInvalid;
  }
  return store.object[key];
}

Variant& Variant::At(size_t idx) {
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

const Variant& Variant::At(size_t idx) const {
  assert(typeno == LIST);
  if (typeno != LIST) {
    return kInvalid;
  }
  if (idx < store.list.size()) {
    return kInvalid;
  }
  return store.list[idx];
}

size_t Variant::Serialize(char* begin, char* end,
                          const SerializeOpts& opts) const {
  BufPrinter printer{begin, end};
  this->Serialize(&printer, opts, 0);
  return printer.Size();
}

void Variant::Serialize(BufPrinter* out, const SerializeOpts& opts,
                        size_t depth) const {
  switch (typeno) {
    case LIST: {
      if (store.list.size() < 1) {
        (*out)("[]");
      } else {
        (*out)("[");
        auto iter = store.list.begin();
        while (iter != store.list.end()) {
          FmtIndent(out, opts.indent, depth + 1);
          iter->Serialize(out, opts, depth + 1);
          ++iter;
          if (iter != store.list.end()) {
            (*out)("%s", opts.separators[1]);
          }
          if (opts.indent) {
            (*out)("\n");
          }
        }
        (*out)("]");
      }
      break;
    }
    case OBJECT: {
      if (store.object.size() < 1) {
        (*out)("{}");
      } else {
        (*out)("{");
        if (opts.indent) {
          (*out)("\n");
        }
        auto iter = store.object.begin();
        while (iter != store.object.end()) {
          FmtIndent(out, opts.indent, depth + 1);
          (*out)("\"%s\"%s", iter->first.c_str(), opts.separators[0]);
          iter->second.Serialize(out, opts, depth + 1);
          ++iter;
          if (iter != store.object.end()) {
            (*out)("%s", opts.separators[1]);
          }
          if (opts.indent) {
            (*out)("\n");
          }
        }
        FmtIndent(out, opts.indent, depth);
        (*out)("}");
      }
      break;
    }
    case STRING: {
      (*out)("\"%s\"", store.string.c_str());
      break;
    }
    case REALNO: {
      (*out)("%f", store.realno);
      break;
    }
    case INTEGER: {
      (*out)("%ld", store.integer);
      break;
    }
    case BOOLEAN: {
      (*out)("%s", store.boolean ? "true" : "false");
      break;
    }
    case JNULL: {
      (*out)("null");
      break;
    }
    default:
      break;
  }
}

}  // namespace variant
}  // namespace json
