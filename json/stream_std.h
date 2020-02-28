#pragma once
// Copyright 2018 Josh Bialkowski <josh.bialkowski@gmail.com>

#include <list>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "json/stream.h"

namespace json {
namespace stream {

// -----------------------------------------------------------------------------
//    High level API
// -----------------------------------------------------------------------------

// Convenience entry point. Will serialize the object twice. Once to get the
// size of the necessary buffer, and second with a buffer of the appropriate
// size.
template <typename T>
std::string Emit(const T& obj, const SerializeOpts& opts=kDefaultOpts);

// -----------------------------------------------------------------------------
//    Parser Helpers
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

// -----------------------------------------------------------------------------
//    Emitter Helpers
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

void EmitValue(const std::string& value, const SerializeOpts& opts,
               size_t depth, BufPrinter* out);

template <class T, class Allocator>
void EmitValue(const std::list<T, Allocator>& value, const SerializeOpts& opts,
               size_t depth, BufPrinter* out);

template <class Key, class Compare, class Allocator>
void EmitValue(const std::set<Key, Compare, Allocator>& value,
               const SerializeOpts& opts, size_t depth, BufPrinter* out);

template <class Key, class T, class Compare, class Allocator>
void EmitValue(const std::map<Key, T, Compare, Allocator>& value,
               const SerializeOpts& opts, size_t depth, BufPrinter* out);

template <class T, class Allocator>
void EmitValue(const std::vector<T, Allocator>& value,
               const SerializeOpts& opts, size_t depth, BufPrinter* out);

}  // namespace stream
}  // namespace json
