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

#include "json/parse.h"

namespace json {
namespace stream {

// -----------------------------------------------------------------------------
//    Parser Helpers
// -----------------------------------------------------------------------------

void ParseString(const Token& token, std::string* value);

// Parse any container that implements the standard insertable concept.
// In particular, `std::insert_iterator(&container, container->begin())`
// can be constructed.
template <typename T>
void ParseInsertable(const Event& entry_event, LexerParser* stream, T* out);

// -----------------------------------------------------------------------------
//    Parser Overloads
// -----------------------------------------------------------------------------

void ParseValue(const Event& event, LexerParser* stream, std::string* value);

template <class T, class Allocator>
void ParseValue(const Event& event, LexerParser* stream,
                std::list<T, Allocator>* value);

template <class Key, class T, class Compare, class Allocator>
void ParseValue(const Event& entry_event, LexerParser* stream,
                std::map<Key, T, Compare, Allocator>* out);

template <class Key, class Compare, class Allocator>
void ParseValue(const Event& event, LexerParser* stream,
                std::set<Key, Compare, Allocator>* value);

template <class T, class Allocator>
void ParseValue(const Event& event, LexerParser* stream,
                std::vector<T, Allocator>* value);

void ParseValue(const Event& event, LexerParser* stream, std::string* value);

}  // namespace stream
}  // namespace json

//
//
//
// -----------------------------------------------------------------------------
//    Template Implementations
// -----------------------------------------------------------------------------
//
//
//

namespace json {
namespace stream {


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

}  // namespace stream
}  // namespace json
