#pragma once
// Copyright 2018 Josh Bialkowski <josh.bialkowski@gmail.com>

#include <list>
#include <map>
#include <set>
#include <string>
#include <vector>

#include <fmt/format.h>
#include <fmt/ostream.h>
#include <glog/logging.h>

#include "json/stream_tpl.h"

namespace json {
namespace stream {

// -----------------------------------------------------------------------------
//    High Level
// -----------------------------------------------------------------------------

template <typename T>
std::string Emit(const T& obj, const SerializeOpts& opts) {
  BufPrinter out1{nullptr, nullptr};
  EmitValue(obj, opts, 0, &out1);
  std::string buf;
  buf.resize(out1.Size() + 1, '\0');
  BufPrinter out2{&buf[0], &buf[buf.size()]};
  EmitValue(obj, opts, 0, &out2);
  buf.resize(out2.Size());
  return buf;
}

// -----------------------------------------------------------------------------
//    Parser Helpers
// -----------------------------------------------------------------------------

template <typename T>
void ParseInsertable(const Event& entry_event, LexerParser* stream, T* out) {
  if (entry_event.typeno != json::Event::LIST_BEGIN) {
    LOG(WARNING) << fmt::format("Can't parse {} as a list at {}:{}",
                                json::Event::ToString(entry_event.typeno),
                                entry_event.token.location.lineno,
                                entry_event.token.location.colno);
    SinkValue(entry_event, stream);
    return;
  }

  // TODO(josh): should we?
  // out->clear();
  std::insert_iterator<T> iter(*out, out->begin());

  json::Event event{};
  json::Error error{};
  while (stream->GetNextEvent(&event, &error) == 0) {
    if (event.typeno == json::Event::LIST_END) {
      return;
    }
    typename T::value_type value{};
    ParseValue(event, stream, &value);
    *(iter++) = value;
  }
  LOG(WARNING) << error.msg;
}

// -----------------------------------------------------------------------------
//    Parse Overloads
// -----------------------------------------------------------------------------

template <class T, class Allocator>
void ParseValue(const Event& event, LexerParser* stream,
                std::vector<T, Allocator>* value) {
  ParseInsertable(event, stream, value);
}

template <class T, class Allocator>
void ParseValue(const Event& event, LexerParser* stream,
                std::list<T, Allocator>* value) {
  ParseInsertable(event, stream, value);
}

template <class Key, class Compare, class Allocator>
void ParseValue(const Event& event, LexerParser* stream,
                std::set<Key, Compare, Allocator>* value) {
  ParseInsertable(event, stream, value);
}

// NOTE(josh): this is very similar to the generic object parser above, but
// the one tricky bit is that it uses ParseValue() to parse the key to a type
// other than a string.
template <class Key, class T, class Compare, class Allocator>
void ParseValue(const Event& entry_event, LexerParser* stream,
                std::map<Key, T, Compare, Allocator>* out) {
  if (entry_event.typeno != json::Event::OBJECT_BEGIN) {
    LOG(WARNING) << fmt::format(
        "Expected JSON object for std::map, but instead got {} at {}:{}",
        json::Event::ToString(entry_event.typeno),
        entry_event.token.location.lineno, entry_event.token.location.colno);
    SinkValue(entry_event, stream);
    return;
  }

  json::Event event;
  json::Error error;
  while (stream->GetNextEvent(&event, &error) == 0) {
    if (event.typeno == json::Event::OBJECT_END) {
      return;
    }

    if (event.typeno != json::Event::OBJECT_KEY) {
      LOG(WARNING) << fmt::format(
          "{}:{} Unexpected {} event at {}:{}", __PRETTY_FUNCTION__, __LINE__,
          json::Event::ToString(event.typeno), event.token.location.lineno,
          event.token.location.colno);
      return;
    }

    // event.typeno == OBJECT_KEY, as expected
    Key key{};
    ParseValue(event, nullptr, &key);
    T& value_ref = (*out)[key];
    if (stream->GetNextEvent(&event, &error) != 0) {
      LOG(WARNING) << error.msg;
      return;
    }
    ParseValue(event, stream, &value_ref);
  }
  LOG(WARNING) << error.msg;
}

// -----------------------------------------------------------------------------
//    Emitter Overloads
// -----------------------------------------------------------------------------

template <class T, class Allocator>
void EmitValue(const std::vector<T, Allocator>& value,
               const SerializeOpts& opts, size_t depth, BufPrinter* out) {
  EmitIterable(value, opts, depth, out);
}

template <class T, class Allocator>
void EmitValue(const std::list<T, Allocator>& value, const SerializeOpts& opts,
               size_t depth, BufPrinter* out) {
  EmitIterable(value, opts, depth, out);
}

template <class Key, class Compare, class Allocator>
void EmitValue(const std::set<Key, Compare, Allocator>& value,
               const SerializeOpts& opts, size_t depth, BufPrinter* out) {
  EmitIterable(value, opts, depth, out);
}

template <class Key, class T, class Compare, class Allocator>
void EmitValue(const std::map<Key, T, Compare, Allocator>& value,
               const SerializeOpts& opts, size_t depth, BufPrinter* out) {
  (*out)("{");
  if (opts.indent) {
    (*out)("\n");
  }
  auto iter = value.begin();
  while (iter != value.end()) {
    FmtIndent(out, opts.indent, depth + 1);
    EmitValue(iter->first, opts, depth + 1, out);
    (*out)(opts.separators[0]);
    EmitValue(iter->second, opts, depth + 1, out);
    ++iter;
    if (iter != value.end()) {
      (*out)("%s", opts.separators[1]);
    }
    if (opts.indent) {
      (*out)("\n");
    }
  }
  FmtIndent(out, opts.indent, depth);
  (*out)("}");
}

}  // namespace stream
}  // namespace json
