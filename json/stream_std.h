#pragma once
// Copyright 2018 Josh Bialkowski <josh.bialkowski@gmail.com>

#include <list>
#include <set>
#include <string>
#include <vector>

#include "json/stream.h"

namespace json {
namespace stream {

// -----------------------------------------------------------------------------
//    Parser Templates
// -----------------------------------------------------------------------------

void ParseString(const Token& token, std::string* value);

// Parse any container that implements the standard insertable concept.
// In particular, `std::insert_iterator(&container, container->begin())`
// can be constructed.
template <typename T>
void ParseInsertable(LexerParser* stream, T* obj);

// -----------------------------------------------------------------------------
//    Parser Overloads
// -----------------------------------------------------------------------------

void Parse(const Event& event, LexerParser* stream, std::string* value);

template <class T, class Allocator>
void Parse(const Event& event, LexerParser* stream,
           std::vector<T, Allocator>* value);

template <class T, class Allocator>
void Parse(const Event& event, LexerParser* stream,
           std::list<T, Allocator>* value);

template <class Key, class Compare, class Allocator>
void Parse(const Event& event, LexerParser* stream,
           std::set<Key, Compare, Allocator>* value);

// -----------------------------------------------------------------------------
//    Emitter Templates
// -----------------------------------------------------------------------------

void EmitString(const std::string& value, BufPrinter* out);

// Emit objects from any container that implements the standard iterable
// concept (i.e. `.begin()` and `.end()` return iterator of some type that
// deferences to `::value_type`.
template <typename T>
void EmitIterable(const T* obj, const SerializeOpts& opts, size_t depth,
                  BufPrinter* out);

// -----------------------------------------------------------------------------
//    Emitter Overloads
// -----------------------------------------------------------------------------

void Emit(const std::string& value, const SerializeOpts& opts, size_t depth,
          BufPrinter* out);

template <class T, class Allocator>
void Emit(const std::vector<T, Allocator>& value, const SerializeOpts& opts,
          size_t depth, BufPrinter* out);

template <class T, class Allocator>
void Emit(const std::list<T, Allocator>& value, const SerializeOpts& opts,
          size_t depth, BufPrinter* out);

template <class Key, class Compare, class Allocator>
void Emit(const std::set<Key, Compare, Allocator>& value,
          const SerializeOpts& opts, size_t depth, BufPrinter* out);

}  // namespace stream
}  // namespace json

//
//
//
// -----------------------------------------------------------------------------
//                       Template Implementations
// -----------------------------------------------------------------------------
//
//
//

namespace json {
namespace stream {

// -----------------------------------------------------------------------------
//    Parser Templates
// -----------------------------------------------------------------------------

template <typename T>
void ParseInsertable(LexerParser* stream, T* obj) {
  typename T::value_type value{};
  ValueParser handler{};
  AssignParser(&handler, &value);
  std::insert_iterator<T> iter(*obj, obj->begin());
  Event event{};
  while (stream->GetNextEvent(&event) >= 0) {
    switch (event.typeno) {
      case Event::LIST_BEGIN:
        break;

      case Event::LIST_END:
        return;

      default:
        value = (*iter);
        handler.Dispatch(event, stream);
        *(iter++) = value;
        break;
    }
  }
}

// -----------------------------------------------------------------------------
//    Parser Overloads
// -----------------------------------------------------------------------------

template <class T, class Allocator>
void Parse(const Event& event, LexerParser* stream,
           std::vector<T, Allocator>* value) {
  ParseInsertable(stream, value);
}

template <class T, class Allocator>
void Parse(const Event& event, LexerParser* stream,
           std::list<T, Allocator>* value) {
  ParseInsertable(stream, value);
}

template <class Key, class Compare, class Allocator>
void Parse(const Event& event, LexerParser* stream,
           std::set<Key, Compare, Allocator>* value) {
  ParseInsertable(stream, value);
}

// -----------------------------------------------------------------------------
//    Emitter Templates
// -----------------------------------------------------------------------------

template <typename T>
void EmitIterable(const T* obj, const SerializeOpts& opts, size_t depth,
                  BufPrinter* out) {
  if (obj->size() < 1) {
    (*out)("[]");
  } else {
    (*out)("[");
    if (opts.indent) {
      (*out)("\n");
    }
    auto iter = obj->begin();
    while (iter != obj->end()) {
      ValueEmitter emitter{};
      AssignEmitter(&emitter, &(*iter));
      FmtIndent(out, opts.indent, depth + 1);
      emitter.Dispatch(opts, depth + 1, out);
      ++iter;
      if (iter != obj->end()) {
        (*out)("%s", opts.separators[1]);
      }
      if (opts.indent) {
        (*out)("\n");
      }
    }
    (*out)("]");
  }
}

// -----------------------------------------------------------------------------
//    Emitter Overloads
// -----------------------------------------------------------------------------

template <class T, class Allocator>
void Emit(const std::vector<T, Allocator>& value, const SerializeOpts& opts,
          size_t depth, BufPrinter* out) {
  EmitIterable(value, opts, depth, out);
}

template <class T, class Allocator>
void Emit(const std::list<T, Allocator>& value, const SerializeOpts& opts,
          size_t depth, BufPrinter* out) {
  EmitIterable(value, opts, depth, out);
}

template <class Key, class Compare, class Allocator>
void Emit(const std::set<Key, Compare, Allocator>& value,
          const SerializeOpts& opts, size_t depth, BufPrinter* out) {
  EmitIterable(value, opts, depth, out);
}

}  // namespace stream
}  // namespace json
