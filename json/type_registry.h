// Copyright 2018 Josh Bialkowski <josh.bialkowski@gmail.com>
#pragma once

#include <map>

#include <fmt/format.h>
#include <fmt/ostream.h>
#include <glog/logging.h>

#include "json/json.h"
#include "json/parse.h"
#include "json/util.h"
#include "util/type_string.h"

namespace json {
namespace stream {

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

enum SerializeAs {
  SCALAR,
  OBJECT,
};

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

  template <typename T>
  int register_object(  //
      int (*parse_fn)(const Registry&, const re2::StringPiece&, LexerParser*,
                      T*) = nullptr,
      int (*dump_fn)(const T&, Dumper*) = nullptr) {
    void* parse_ptr = reinterpret_cast<void*>(parse_fn);
    void* dump_ptr = reinterpret_cast<void*>(dump_fn);
    parsers_[get_key<T>()] = SerializeSpec{OBJECT, parse_ptr, dump_ptr};
    return 0;
  }

  template <typename T>
  int register_scalar(  //
      void (*parse_fn)(const Token&, T*) = nullptr,
      int (*dump_fn)(Dumper*, const T&) = nullptr) {
    void* parse_ptr = reinterpret_cast<void*>(parse_fn);
    void* dump_ptr = reinterpret_cast<void*>(dump_fn);
    parsers_[get_key<T>()] = SerializeSpec{SCALAR, parse_ptr, dump_ptr};
    return 0;
  }

  template <class T>
  int parse_array(LexerParser* event_stream, T* begin, T* end) const {
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

  template <class T, size_t N>
  int parse_array(LexerParser* event_stream, T (*arr)[N]) const {
    return parse_array<T>(event_stream, &(*arr)[0], &(*arr)[N]);
  }

  // Walk the JSON event stream for the current object and dispatch the field
  // parser for each key, value pair in the JSON event stream.
  template <typename T>
  int parse_object(LexerParser* event_stream, T* out) const {
    json::Event event;
    json::Error error;
    if (event_stream->GetNextEvent(&event, &error)) {
      LOG(WARNING) << fmt::format(
          "In {}, Failed to get JSON object start event", __PRETTY_FUNCTION__);
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
      LOG(WARNING)
          << "No parser registered for type '" << type_string<T>()
          << "', skipping the parse. Program will probably crash later!";
      SinkValue(event, event_stream);
    }
    typedef int (*ParseFieldFn)(const Registry&, const re2::StringPiece&,
                                LexerParser*, T*);

    ParseFieldFn parse_field =
        reinterpret_cast<ParseFieldFn>(iter->second.parse_fun);

    while (event_stream->GetNextEvent(&event, &error) == 0) {
      if (event.typeno == json::Event::OBJECT_END) {
        return error.code;
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
  int parse_scalar(LexerParser* event_stream, T* out) const {
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
      LOG(WARNING)
          << "No parser registered for type '" << type_string<T>()
          << "', skipping the parse. Program will probably crash later!";
      return 1;
    }
    typedef void (*ParseTokenFn)(const Token&, T*);
    ParseTokenFn parse_token =
        reinterpret_cast<ParseTokenFn>(iter->second.parse_fun);
    parse_token(event.token, out);
    return 0;
  }

  template <typename T>
  int parse_value(LexerParser* stream, T* out) const {
    void* key = get_key<T>();
    auto iter = parsers_.find(key);
    if (iter == parsers_.end()) {
      LOG(WARNING)
          << "No parser registered for type '" << type_string<T>()
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
  int parse_value(LexerParser* event_stream, T (*out)[N]) const {
    return this->parse_array(event_stream, out);
  }

  template <typename Iterator>
  int dump_list(Dumper* dumper, Iterator begin, Iterator end) const;

  template <class T, size_t N>
  int dump_list(Dumper* dumper, T (&arr)[N]) const {
    return dump_list<T>(dumper, &arr[0], &arr[N]);
  }

  template <typename T>
  int dump_object(Dumper* dumper, const T& out) const;

  template <typename T>
  int dump_scalar(Dumper* dumper, const T& out) const;

  template <size_t N>
  int dump_scalar(Dumper* dumper, const char (&out)[N]) const;

  template <typename T>
  int dump_value(Dumper* dumper, const T& out) const;

 private:
  std::map<void*, SerializeSpec> parsers_;
};

Registry* global_registry();

template <typename T>
int parse(LexerParser* event_stream, T* out,
          const Registry* registry = nullptr) {
  if (!registry) {
    registry = global_registry();
  }
  registry->parse_value(event_stream, out);
  return 0;
}

template <typename T>
int parse(const re2::StringPiece& content, T* out,
          const Registry* registry = nullptr) {
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

struct DumpEvent {
  enum TypeNo {
    OBJECT_BEGIN,
    OBJECT_KEY,
    OBJECT_END,
    LIST_BEGIN,
    LIST_END,
    VALUE,
    INVALID,
  };

  TypeNo typeno;
  static const char* to_string(TypeNo value);
};

class Dumper {
 public:
  explicit Dumper(Registry* registry) : registry_{registry} {
    if (!registry) {
      registry_ = global_registry();
    }
  }

  // Push an event notification (e.g. semantic boundaries) to the output
  virtual void dump_event(DumpEvent::TypeNo eventno) = 0;

  template <class T>
  int dump_field(const re2::StringPiece& key, const T& value) {
    int result = 0;
    this->dump_event(DumpEvent::OBJECT_KEY);
    result |= registry_->dump_scalar(this, key);
    this->dump_event(DumpEvent::VALUE);
    result |= registry_->dump_value(this, value);
    return result;
  }

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

 private:
  Registry* registry_;
};

enum GuardType {
  GUARD_OBJECT,
  GUARD_LIST,
};

// Scope guard for begin/end pairs
class DumpGuard {
 public:
  DumpGuard(Dumper* dumper, GuardType type) : dumper_(dumper), type_(type) {
    switch (type_) {
      case GUARD_OBJECT:
        dumper_->dump_event(DumpEvent::OBJECT_BEGIN);
        break;
      case GUARD_LIST:
        dumper_->dump_event(DumpEvent::LIST_BEGIN);
        break;
    }
  }

  ~DumpGuard() {
    switch (type_) {
      case GUARD_OBJECT:
        dumper_->dump_event(DumpEvent::OBJECT_END);
        break;
      case GUARD_LIST:
        dumper_->dump_event(DumpEvent::LIST_END);
        break;
    }
  }

 private:
  Dumper* dumper_;
  GuardType type_;
};

template <typename Iterator>
int Registry::dump_list(Dumper* dumper, Iterator begin, Iterator end) const {
  DumpGuard _guard{dumper, GUARD_LIST};

  for (Iterator iter = begin; iter != end; iter++) {
    dumper->dump_event(DumpEvent::VALUE);
    if (this->dump_value(*iter)) {
      LOG(WARNING) << "Element dump failed for list with iterator type"
                   << type_string<Iterator>()
                   << "', dumping empty list. Parse will probably fail later!";
      return 1;
    }
  }
}

template <typename T>
int Registry::dump_object(Dumper* dumper, const T& obj) const {
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
int Registry::dump_scalar(Dumper* dumper, const T& value) const {
  void* key = get_key<T>();
  auto iter = parsers_.find(key);
  if (iter == parsers_.end()) {
    LOG(WARNING) << "No formatter registered for type '" << type_string<T>()
                 << "', dumping empty object. Parse will probably fail later!";
    return 1;
  }
  typedef int (*DumpScalarFn)(Dumper*, const T&);
  DumpScalarFn dump_scalar =
      reinterpret_cast<DumpScalarFn>(iter->second.dump_fun);
  if (!dump_scalar) {
    LOG(WARNING) << "No formatter registered for type '" << type_string<T>()
                 << "', dumping empty object. Parse will probably fail later!";
    return 1;
  }

  if (dump_scalar(dumper, value)) {
    LOG(WARNING) << "Failed to format type " << type_string<T>()
                 << " into output buffer. Possibly there isn't enough space.";
    return 1;
  }
  return 0;
}

template <size_t N>
int Registry::dump_scalar(Dumper* dumper, const char (&str)[N]) const {
  return this->dump_scalar(dumper, re2::StringPiece(str));
}

template <typename T>
int Registry::dump_value(Dumper* dumper, const T& value) const {
  void* key = get_key<T>();
  auto iter = parsers_.find(key);
  if (iter == parsers_.end()) {
    LOG(WARNING) << "No dumper registered for type '" << type_string<T>()
                 << "', dumping null. Parse will probably fail!";
    dumper->dump_primitive(nullptr);
    return 1;
  }
  if (iter->second.parse_as == SCALAR) {
    return this->dump_scalar(dumper, value);
  } else {
    return this->dump_object(dumper, value);
  }
}

struct DumpStack {
  enum TypeNo {
    OBJECT,
    LIST,
    FIELD,
  };

  TypeNo type;
  uint32_t count;
};

class StreamDumper : public Dumper {
 public:
  // NOLINTNEXTLINE
  StreamDumper(std::ostream* ostream, Registry* registry = nullptr)
      : Dumper(registry), ostream_{ostream} {}

  virtual ~StreamDumper() {}

  void dump_event(DumpEvent::TypeNo eventno) override {
    switch (eventno) {
      case DumpEvent::LIST_BEGIN:
      case DumpEvent::OBJECT_BEGIN:
      case DumpEvent::VALUE:
        if (dump_stack_.size() && dump_stack_.back().type == DumpStack::FIELD) {
          (*ostream_) << " : ";
          dump_stack_.pop_back();
        }
        break;
      default:
        break;
    }

    switch (eventno) {
      case DumpEvent::LIST_BEGIN:
        (*ostream_) << "[";
        dump_stack_.push_back({DumpStack::LIST, 0});
        break;
      case DumpEvent::LIST_END:
        (*ostream_) << "]";
        // assert dump_stack_.back().type_ == DumpStack::LIST
        dump_stack_.pop_back();
        break;
      case DumpEvent::OBJECT_BEGIN:
        (*ostream_) << "{";
        dump_stack_.push_back({DumpStack::OBJECT, 0});
        break;
      case DumpEvent::OBJECT_END:
        (*ostream_) << "}";
        // assert dump_stack_.back().type_ == DumpStack::OBJECT
        dump_stack_.pop_back();
        break;
      case DumpEvent::OBJECT_KEY:
        // assert dump_sack.size()
        // assert dump_stack_.back().type_ == DumpStack::OBJECT
        if (dump_stack_.back().count > 0) {
          (*ostream_) << ", ";
        }
        dump_stack_.push_back({DumpStack::FIELD, 0});
        break;
      case DumpEvent::VALUE:
        dump_stack_.back().count += 1;
        break;
      default:
        break;
    }
  }

  void dump_primitive(uint8_t value) override {
    (*ostream_) << value;
  };
  void dump_primitive(uint16_t value) override {
    (*ostream_) << value;
  };
  void dump_primitive(uint32_t value) override {
    (*ostream_) << value;
  };
  void dump_primitive(uint64_t value) override {
    (*ostream_) << value;
  };
  void dump_primitive(int8_t value) override {
    (*ostream_) << value;
  };
  void dump_primitive(int16_t value) override {
    (*ostream_) << value;
  };
  void dump_primitive(int32_t value) override {
    (*ostream_) << value;
  };
  void dump_primitive(int64_t value) override {
    (*ostream_) << value;
  };
  void dump_primtiive(float value) override {
    (*ostream_) << value;
  };
  void dump_primitive(double value) override {
    (*ostream_) << value;
  };
  void dump_primitive(bool value) override {
    (*ostream_) << (value ? "true" : "false");
  };
  void dump_primitive(std::nullptr_t nullval) override {
    (*ostream_) << "null";
  };
  void dump_primitive(re2::StringPiece strval) override {
    (*ostream_) << '\"';
    escape(strval, ostream_);
    (*ostream_) << '\"';
  };
  void dump_primitive(const std::string& strval) override {
    (*ostream_) << '\"';
    escape(strval, ostream_);
    (*ostream_) << '\"';
  };
  void dump_primitive(const char* strval) override {
    (*ostream_) << '\"';
    escape(strval, ostream_);
    (*ostream_) << '\"';
  };

 private:
  std::ostream* ostream_;
  std::array<char, 1024> buffer_;
  std::vector<DumpStack> dump_stack_;
};

template <typename T>
int dump(Dumper* dumper, const T& value, Registry* registry = nullptr) {
  if (!registry) {
    registry = global_registry();
  }
  registry->dump_value(dumper, value);
  return 0;
}

template <typename T>
std::string dump(const T& value, Registry* registry = nullptr) {
  if (!registry) {
    registry = global_registry();
  }
  std::stringstream strstream;
  StreamDumper dumper{&strstream, registry};
  registry->dump_value(dumper, value);
  return strstream.str();
}

}  // namespace stream
}  // namespace json
