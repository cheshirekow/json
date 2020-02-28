#pragma once
// Copyright 2018 Josh Bialkowski <josh.bialkowski@gmail.com>

#include <cassert>
#include <map>

#include <fmt/format.h>
#include <fmt/ostream.h>
#include <glog/logging.h>

#include "json/json.h"
#include "json/parse.h"
#include "json/util.h"
#include "util/stack_trace.h"
#include "util/type_string.h"

namespace json {
namespace stream {

// ----------------------------------------------------------------------------
//    Registry API
// ----------------------------------------------------------------------------

// Empty sentinel function template. Instanciations of this template provide a
// unique address which can be used to key off the type T in a type registry.
template <class T>
void sentinel_function(T*) {}

// Return a unique key for the given type.
template <typename T>
void* get_key() {
  void (*dummy_instance)(T*) = &sentinel_function<T>;
  return reinterpret_cast<void*>(dummy_instance);
}

// Indicates whether a particular entry is for serialization of an aggregate
// (JSON object, C/C++ structure) or a scalar (primitive, native type).
enum SerializeAs {
  SCALAR,
  OBJECT,
};

// The value type for the registry mapping. Stores the parse and dump functions
// for the registered type, as well as a flag indicating how to interpret the
// signatures for those function pointers.
struct SerializeSpec {
  SerializeAs parse_as;
  void* parse_fun;
  void* dump_fun;
};

class Dumper;

// Stores function pointers for parser, formatter, and dumper helper functions
// for JSON-serializable types.
class Registry {
 public:
  // Registers all built-in serializable types
  Registry();
  ~Registry() {}

  // Registry a JSON-object serializable type.
  template <typename T>
  int register_object(  //
      int (*parse_fn)(const Registry&, const re2::StringPiece&, LexerParser*,
                      T*) = nullptr,
      int (*dump_fn)(const T&, Dumper*) = nullptr);

  // Register a JSON-scalar serializable type.
  template <typename T>
  int register_scalar(  //
      int (*parse_fn)(const Token&, T*) = nullptr,
      int (*dump_fn)(const T&, Dumper*) = nullptr);

  // Parse a list of JSON-serializable types into a fixed buffer of objects
  // pointed two by the pair of iterators.
  // TODO(josh): can these be templated iterators?
  template <class T>
  int parse_list(LexerParser* event_stream, T* begin, T* end) const;

  // Specialization for a fixed-sized array.
  template <class T, size_t N>
  int parse_list(LexerParser* event_stream, T (*arr)[N]) const {
    return parse_list<T>(event_stream, &(*arr)[0], &(*arr)[N]);
  }

  // Walk the JSON event stream for the current object and dispatch the field
  // parser for each key, value pair in the JSON event stream.
  template <typename T>
  int parse_object(LexerParser* event_stream, T* out) const;

  // Extract a value token from the event stream and pass it to the registered
  // token parser for the given object type.
  template <typename T>
  int parse_scalar(LexerParser* event_stream, T* out) const;

  // Lookup the serialization type (object,scalar) for the given object and
  // then dispatch either parse_object or parse_scalar
  template <typename T>
  int parse_value(LexerParser* stream, T* out) const;

  // Specialization for a fixed-size array: dispatch the array parser
  template <typename T, size_t N>
  int parse_value(LexerParser* event_stream, T (*out)[N]) const;

  template <typename Iterator>
  int dump_list(Iterator begin, Iterator end, Dumper* dumper) const;

  template <class T, size_t N>
  int dump_list(const T (&arr)[N], Dumper* dumper) const {
    return dump_list(&arr[0], &arr[N], dumper);
  }

  // Dump a JSON-object to the dumper:
  // 1. Lookup the dumpfields helper,
  // 2. dump the object start event,
  // 3. walk the field list dumping each field (recursively)
  // 4. dump an object end event.
  template <typename T>
  int dump_object(const T& out, Dumper* dumper) const;

  // Dump a JSON-scalar to the dumper. Lookup the output-type conversion
  // function registered for this object type, do the conversion, and then
  // call into one of the dumpers primitive functions.
  template <typename T>
  int dump_scalar(const T& out, Dumper* dumper) const;

  // Specialization for string output. Convert the character array to a string
  // type and then dump that
  template <size_t N>
  int dump_scalar(const char (&out)[N], Dumper* dumper) const;

  // Lookup the serialization type (object, scalar) for the given value and
  // then dispatch either dump_object or dump_scalar.
  template <typename T>
  int dump_value(const T& out, Dumper* dumper) const;

  // Specialization for a fixed-sized array: dispatch the list dumper
  template <class T, size_t N>
  int dump_value(const T (&arr)[N], Dumper* dumper) const {
    return this->dump_list(arr, dumper);
  }

  // Specialization for fixed-sized strings: dispatch dump_scalar
  template <size_t N>
  int dump_value(const char (&arr)[N], Dumper* dumper) const {
    return this->dump_scalar(arr, dumper);
  }

 private:
  // Stores a map from the type-key to the specification of the dump/parse
  // functions for that type.
  std::map<void*, SerializeSpec> parsers_;
};

// ----------------------------------------------------------------------------
//    Dumper API
// ----------------------------------------------------------------------------

// Enumeration for the different semantic events that we might notify the
// dumper about
struct DumpEvent {
  enum TypeNo {
    OBJECT_BEGIN,
    OBJECT_KEY,
    OBJECT_VALUE,
    OBJECT_END,
    LIST_BEGIN,
    LIST_END,
    LIST_VALUE,
    INVALID,
  };

  TypeNo typeno;
  static const char* to_string(TypeNo value);
};

// Interface for JSON output formatters. Can also be implemented for a kind of
// poor-mans introspection on your serializable types.
class Dumper {
 public:
  explicit Dumper(const Registry* registry, const SerializeOpts& opts);

  // Push an event notification (e.g. semantic boundaries) to the output
  virtual void dump_event(DumpEvent::TypeNo eventno) = 0;

  // Dump a field given it's name (as a string) and value. Pushes event
  // notifications for the field key and value followed by the actual
  // string and value.
  template <class T>
  int dump_field(const re2::StringPiece& key, const T& value);

  virtual void dump_primitive(uint8_t value) = 0;
  virtual void dump_primitive(uint16_t value) = 0;
  virtual void dump_primitive(uint32_t value) = 0;
  virtual void dump_primitive(uint64_t value) = 0;
  virtual void dump_primitive(int8_t value) = 0;
  virtual void dump_primitive(int16_t value) = 0;
  virtual void dump_primitive(int32_t value) = 0;
  virtual void dump_primitive(int64_t value) = 0;
  virtual void dump_primtiive(float value) = 0;
  virtual void dump_primitive(double value) = 0;
  virtual void dump_primitive(bool value) = 0;
  virtual void dump_primitive(std::nullptr_t nullval) = 0;
  virtual void dump_primitive(re2::StringPiece strval) = 0;
  virtual void dump_primitive(const std::string& strval) = 0;
  virtual void dump_primitive(const char* strval) = 0;

 protected:
  const Registry* registry_;  //< registry of dump/parse implementations
  SerializeOpts opts_;        //< options for how to format the output
};

// Indicates the type of scope guard
enum GuardType {
  GUARD_OBJECT,
  GUARD_LIST,
};

// Scope guard for begin/end pairs
class DumpGuard {
 public:
  // Push the begin-event notification to the dumper
  DumpGuard(Dumper* dumper, GuardType type);

  // Push the end-event notification to the dumper
  ~DumpGuard();

 private:
  Dumper* dumper_;
  GuardType type_;
};

// Indicates the type of "aggregate" which is currently open at each level in
// the current stack of dump calls.
struct DumpStack {
  enum TypeNo {
    OBJECT,
    LIST,
    FIELD,
  };

  TypeNo type;
  uint32_t count;
};

// Dumper implementation which writes output to a std::ostream using stream
// operators.
class StreamDumper : public Dumper {
 public:
  // NOLINTNEXTLINE
  StreamDumper(std::ostream* ostream, const SerializeOpts& opts = kDefaultOpts,
               const Registry* registry = nullptr);
  virtual ~StreamDumper() {}

  void dump_event(DumpEvent::TypeNo eventno) override;
  void dump_primitive(uint8_t value) override;
  void dump_primitive(uint16_t value) override;
  void dump_primitive(uint32_t value) override;
  void dump_primitive(uint64_t value) override;
  void dump_primitive(int8_t value) override;
  void dump_primitive(int16_t value) override;
  void dump_primitive(int32_t value) override;
  void dump_primitive(int64_t value) override;
  void dump_primtiive(float value) override;
  void dump_primitive(double value) override;
  void dump_primitive(bool value) override;
  void dump_primitive(std::nullptr_t nullval) override;
  void dump_primitive(re2::StringPiece strval) override;
  void dump_primitive(const std::string& strval) override;
  void dump_primitive(const char* strval) override;

 private:
  std::ostream* ostream_;
  std::array<char, 1024> buffer_;
  std::vector<DumpStack> dump_stack_;
};

// ----------------------------------------------------------------------------
//    High level convenience functions
// ----------------------------------------------------------------------------

// Return a pointer to a program-singleton global registry object. Note that
// the object is constructed on the first call. This is safe to call from
// static initialization context.
Registry* global_registry();

// Parse the given event stream into the given object using the specified
// function registry (default is the global registry).
template <typename T>
int parse(LexerParser* event_stream, T* out,
          const Registry* registry = nullptr);

// Parse a JSON-document (encoded as a StringPiece) into the given object
// using the specified function registry (default is the global registry).
template <typename T>
int parse(const re2::StringPiece& content, T* out,
          const Registry* registry = nullptr);

// Dump the given value to a JSON document using the specified dumper and
// the specified type registry (default is the global registry)
template <typename T>
int dump(Dumper* dumper, const T& value, const Registry* registry = nullptr);

// Dump the given value to a JSON document writing it to the provided output
// stream and using the specified format options and type registry (default is
// the global registry)
template <typename T>
int dump(const T& value, std::ostream* stream,
         const json::SerializeOpts& opts = kDefaultOpts,
         const Registry* registry = nullptr);

// Dump the given value to a JSON document and return the document as a string
// formatted with the given serialization options and using the specified
// type registry (default is the global registry)
template <typename T>
std::string dump(const T& value, const json::SerializeOpts& opts = kDefaultOpts,
                 const Registry* registry = nullptr);

}  // namespace stream
}  // namespace json

//
//
//
// ============================================================================
//     Template Implementations
// ============================================================================
//
//
//

namespace json {
namespace stream {

// -----------------------------------------------------------------------------
//    class Registry
// -----------------------------------------------------------------------------

template <typename T>
int Registry::register_object(  //
    int (*parse_fn)(const Registry&, const re2::StringPiece&, LexerParser*, T*),
    int (*dump_fn)(const T&, Dumper*)) {
  void* parse_ptr = reinterpret_cast<void*>(parse_fn);
  void* dump_ptr = reinterpret_cast<void*>(dump_fn);
  parsers_[get_key<T>()] = SerializeSpec{OBJECT, parse_ptr, dump_ptr};
  return 0;
}

template <typename T>
int Registry::register_scalar(  //
    int (*parse_fn)(const Token&, T*), int (*dump_fn)(const T&, Dumper*)) {
  void* parse_ptr = reinterpret_cast<void*>(parse_fn);
  void* dump_ptr = reinterpret_cast<void*>(dump_fn);
  parsers_[get_key<T>()] = SerializeSpec{SCALAR, parse_ptr, dump_ptr};
  return 0;
}

template <class T>
int Registry::parse_list(LexerParser* event_stream, T* begin, T* end) const {
  Event event{};
  Error error{};
  if (event_stream->GetNextEvent(&event, &error)) {
    LOG(WARNING) << fmt::format("In {}, Failed to get JSON list start event",
                                __PRETTY_FUNCTION__);
    return error.code;
  }
  if (event.typeno != json::Event::LIST_BEGIN) {
    LOG(WARNING) << fmt::format(
        "In {}, expected JSON list of {}, but instead got {} at {}:{}",
        __PRETTY_FUNCTION__, type_string<T>(),
        json::Event::ToString(event.typeno), event.token.location.lineno,
        event.token.location.colno);
    SinkValue(event, event_stream);
    return 1;
  }

  for (T* iter = begin; iter != end; iter++) {
    // NOTE(josh): this is bad, we can't distinguish between an error and
    // a stop-iteration.
    if (this->parse_value(event_stream, iter)) {
      return 0;
    }
  }
  if (event_stream->GetNextEvent(&event, &error)) {
    LOG(WARNING) << fmt::format(
        "In {}, Failed to get JSON list item or end event",
        __PRETTY_FUNCTION__);
    return error.code;
  }
  if (event.typeno != Event::LIST_END) {
    LOG(WARNING) << "skipping array elements after "
                 << static_cast<size_t>(end - begin);
  }
  // NOTE(josh): this is also bad, because we can't distinguish between
  // errors and list-termination.
  while (event.typeno != Event::LIST_END) {
    SinkValue(event, event_stream);
    if (event_stream->GetNextEvent(&event, &error)) {
      LOG(WARNING) << fmt::format(
          "In {}, Failed to get JSON list item or end event",
          __PRETTY_FUNCTION__);
      return error.code;
    }
  }
  return 0;
}

template <typename T>
int Registry::parse_object(LexerParser* event_stream, T* out) const {
  json::Event event;
  json::Error error;
  if (event_stream->GetNextEvent(&event, &error)) {
    LOG(WARNING) << fmt::format("In {}, Failed to get JSON object start event",
                                __PRETTY_FUNCTION__);
    return error.code;
  }
  if (event.typeno != json::Event::OBJECT_BEGIN) {
    if (event.typeno != json::Event::LIST_END) {
      // If we encounter a LIST_END but there is no `error` then it must
      // mean that we are in the process of parsing a list. In that case
      // we return 1 so that the list parser terminates it's loop, but
      // we don't warn or try to sink anything because this is not an error
      LOG(WARNING) << fmt::format(
          "Expected JSON object for {}, but instead got {} at {}:{}",
          type_string<T>(), json::Event::ToString(event.typeno),
          event.token.location.lineno, event.token.location.colno);
      SinkValue(event, event_stream);
    }
    return 1;
  }

  void* key = get_key<T>();
  auto iter = parsers_.find(key);
  if (iter == parsers_.end()) {
    LOG(WARNING) << "No parser registered for type '" << type_string<T>()
                 << "', skipping the parse. Program will probably crash later!";
    SinkValue(event, event_stream);
  }
  typedef int (*ParseFieldFn)(const Registry&, const re2::StringPiece&,
                              LexerParser*, T*);

  ParseFieldFn parse_field =
      reinterpret_cast<ParseFieldFn>(iter->second.parse_fun);

  while (event_stream->GetNextEvent(&event, &error) == 0) {
    if (event.typeno == json::Event::OBJECT_END) {
      return 0;
    }

    if (event.typeno != json::Event::OBJECT_KEY) {
      LOG(WARNING) << fmt::format(
          "{}:{} Unexpected {} event at {}:{}", __PRETTY_FUNCTION__, __LINE__,
          json::Event::ToString(event.typeno), event.token.location.lineno,
          event.token.location.colno);
      return 1;
    }

    // event.typeno == OBJECT_KEY, as expected
    Token keytoken = event.token;

    // NOTE(josh): strip literal quotes off of string token
    re2::StringPiece keyvalue =
        keytoken.spelling.substr(1, keytoken.spelling.size() - 2);

    if (parse_field(*this, keyvalue, event_stream, out)) {
      LOG(WARNING) << fmt::format(
          "Unrecognized key {}({}) at {}:{}", keyvalue, RuntimeHash(keyvalue),
          keytoken.location.lineno, keytoken.location.colno);
    }
  }
  LOG(WARNING) << error.msg;
  return error.code;
}

template <typename T>
int Registry::parse_scalar(LexerParser* event_stream, T* out) const {
  json::Event event;
  json::Error error;
  if (event_stream->GetNextEvent(&event, &error)) {
    LOG(WARNING) << fmt::format("In {}, Failed to get JSON scalar event",
                                __PRETTY_FUNCTION__);
    return error.code;
  }
  if (event.typeno != json::Event::VALUE_LITERAL) {
    if (event.typeno != json::Event::LIST_END) {
      // If we encounter a LIST_END but there is no `error` then it must
      // mean that we are in the process of parsing a list. In that case
      // we return 1 so that the list parser terminates it's loop, but
      // we don't warn or try to sink anything because this is not an error
      LOG(WARNING) << fmt::format(
          "Expected JSON scalar of type {}, but instead got {} at {}:{}",
          type_string<T>(), json::Event::ToString(event.typeno),
          event.token.location.lineno, event.token.location.colno);
      SinkValue(event, event_stream);
    }
    return 1;
  }

  void* key = get_key<T>();
  auto iter = parsers_.find(key);
  if (iter == parsers_.end()) {
    LOG(WARNING) << "No parser registered for type '" << type_string<T>()
                 << "', skipping the parse. Program will probably crash later!";
    return 1;
  }
  typedef int (*ParseTokenFn)(const Token&, T*);
  ParseTokenFn parse_token =
      reinterpret_cast<ParseTokenFn>(iter->second.parse_fun);
  return parse_token(event.token, out);
}

template <typename T>
int Registry::parse_value(LexerParser* stream, T* out) const {
  void* key = get_key<T>();
  auto iter = parsers_.find(key);
  if (iter == parsers_.end()) {
    LOG(WARNING) << "No parser registered for type '" << type_string<T>()
                 << "', skipping the parse. Program will probably crash later!";
    SinkValue(stream);
  }
  if (iter->second.parse_as == SCALAR) {
    return this->parse_scalar(stream, out);
  } else {
    return this->parse_object(stream, out);
  }
}

template <typename T, size_t N>
int Registry::parse_value(LexerParser* event_stream, T (*out)[N]) const {
  if (std::is_same<T, char>::value) {
    // NOTE(josh): temporary hack to get around the fact that, evidently,
    //
    // template <size_t N>
    // int parse_value(LexerParser* event_stream, char (*out)[N]) const;
    //
    // is not "more specific" than this template. When we include that
    // function, this one is called anyway.
    std::string temp;
    int result = this->parse_scalar(event_stream, &temp);
    if (result) {
      return result;
    }
    // NOTE(josh): reinterpret_cast is a no-op on the only active code path
    // through this branch.
    char(*write)[N] = reinterpret_cast<char(*)[N]>(out);
    size_t ncopy = temp.copy(*write, N);
    if (ncopy == N) {
      ncopy = N - 1;
    }
    (*write)[ncopy] = '\0';
    return 0;
  }
  return this->parse_list(event_stream, out);
}

template <typename Iterator>
int Registry::dump_list(Iterator begin, Iterator end, Dumper* dumper) const {
  DumpGuard _guard{dumper, GUARD_LIST};

  for (Iterator iter = begin; iter != end; iter++) {
    dumper->dump_event(DumpEvent::LIST_VALUE);
    if (this->dump_value(*iter, dumper)) {
      LOG(WARNING) << "Element dump failed for list with iterator type"
                   << type_string<Iterator>()
                   << "', dumping empty list. Parse will probably fail later!";
      return 1;
    }
  }
  return 0;
}

template <typename T>
int Registry::dump_object(const T& obj, Dumper* dumper) const {
  DumpGuard _guard{dumper, GUARD_OBJECT};

  void* key = get_key<T>();
  auto iter = parsers_.find(key);
  if (iter == parsers_.end()) {
    LOG(WARNING) << "No sepcificationfor type '" << type_string<T>()
                 << "', dumping empty object. Parse will probably fail later!";
    return 1;
  }
  typedef int (*DumpFieldsFn)(const T&, Dumper* dumper);
  DumpFieldsFn dump_fields =
      reinterpret_cast<DumpFieldsFn>(iter->second.dump_fun);
  if (!dump_fields) {
    LOG(WARNING) << "No dumper registered for type '" << type_string<T>()
                 << "', dumping empty object. Parse will probably fail later!";
    return 1;
  }
  dump_fields(obj, dumper);
  return 0;
}

template <typename T>
int Registry::dump_scalar(const T& value, Dumper* dumper) const {
  void* key = get_key<T>();
  auto iter = parsers_.find(key);
  if (iter == parsers_.end()) {
    LOG(WARNING) << "No formatter registered for type '" << type_string<T>()
                 << "', dumping empty object. Parse will probably fail later!";
    return 1;
  }
  typedef int (*DumpScalarFn)(const T&, Dumper*);
  DumpScalarFn dump_token =
      reinterpret_cast<DumpScalarFn>(iter->second.dump_fun);
  if (!dump_token) {
    LOG(WARNING) << "No formatter registered for type '" << type_string<T>()
                 << "', dumping empty object. Parse will probably fail later!";
    return 1;
  }

  if (dump_token(value, dumper)) {
    LOG(WARNING) << "Failed to format type " << type_string<T>()
                 << " into output buffer. Possibly there isn't enough space.";
    return 1;
  }
  return 0;
}

template <size_t N>
int Registry::dump_scalar(const char (&str)[N], Dumper* dumper) const {
  return this->dump_scalar(re2::StringPiece(str), dumper);
}

template <typename T>
int Registry::dump_value(const T& value, Dumper* dumper) const {
  void* key = get_key<T>();
  auto iter = parsers_.find(key);
  if (iter == parsers_.end()) {
    LOG(WARNING) << "No dumper registered for type '" << type_string<T>()
                 << "', dumping null. Parse will probably fail!";
    dumper->dump_primitive(nullptr);
    return 1;
  }
  if (iter->second.parse_as == SCALAR) {
    return this->dump_scalar(value, dumper);
  } else {
    return this->dump_object(value, dumper);
  }
}

// -----------------------------------------------------------------------------
//    class Dumper
// -----------------------------------------------------------------------------

template <class T>
int Dumper::dump_field(const re2::StringPiece& key, const T& value) {
  int result = 0;
  this->dump_event(DumpEvent::OBJECT_KEY);
  result |= registry_->dump_scalar(key, this);
  this->dump_event(DumpEvent::OBJECT_VALUE);
  result |= registry_->dump_value(value, this);
  return result;
}

// -----------------------------------------------------------------------------
//    High level convenience functions
// -----------------------------------------------------------------------------

template <typename T>
int parse(LexerParser* event_stream, T* out, const Registry* registry) {
  if (!registry) {
    registry = global_registry();
  }
  registry->parse_value(event_stream, out);
  return 0;
}

template <typename T>
int parse(const re2::StringPiece& content, T* out, const Registry* registry) {
  if (!registry) {
    registry = global_registry();
  }
  LexerParser event_stream{};
  Error error{};
  if (event_stream.Init(&error)) {
    LOG(WARNING) << error.msg;
    return error.code;
  }
  if (event_stream.Begin(content)) {
    LOG(WARNING) << error.msg;
    return error.code;
  }
  return parse(&event_stream, out, registry);
}

template <typename T>
int dump(Dumper* dumper, const T& value, const Registry* registry) {
  if (!registry) {
    registry = global_registry();
  }
  registry->dump_value(value, dumper);
  return 0;
}

template <typename T>
int dump(const T& value, std::ostream* stream, const json::SerializeOpts& opts,
         const Registry* registry) {
  if (!registry) {
    registry = global_registry();
  }
  StreamDumper dumper{stream, opts, registry};
  return registry->dump_value(value, &dumper);
}

template <typename T>
std::string dump(const T& value, const json::SerializeOpts& opts,
                 const Registry* registry) {
  if (!registry) {
    registry = global_registry();
  }
  std::stringstream strstream;
  dump(value, &strstream, opts, registry);
  return strstream.str();
}

}  // namespace stream
}  // namespace json
