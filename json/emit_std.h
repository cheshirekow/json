#pragma once
// Copyright 2018 Josh Bialkowski <josh.bialkowski@gmail.com>

#include <list>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "json/parse_std.h"
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
std::string Emit(const T& obj, const SerializeOpts& opts = kDefaultOpts);

// -----------------------------------------------------------------------------
//    Emitter Helpers
// -----------------------------------------------------------------------------

void EmitString(const std::string& value, BufPrinter* out);

// -----------------------------------------------------------------------------
//    EmitValue Overloads
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
