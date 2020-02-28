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

#include "json/emit_std.h"
#include "json/parse_std.h"
#include "json/stream.h"

namespace json {
namespace stream {

// -----------------------------------------------------------------------------
//    WalkValue Overloads
// -----------------------------------------------------------------------------

template <class U>
void WalkValue(const std::string& value, const WalkOpts& opts, U* walker);

template <class T, class Allocator, class U>
void WalkValue(const std::list<T, Allocator>& value, const WalkOpts& opts,
               U* walker);

template <class Key, class Compare, class Allocator, class U>
void WalkValue(const std::set<Key, Compare, Allocator>& value,
               const WalkOpts& opts, U* walker);

template <class Key, class T, class Compare, class Allocator, class U>
void WalkValue(const std::map<Key, T, Compare, Allocator>& value,
               const WalkOpts& opts, U* walker);

template <class T, class Allocator, class U>
void WalkValue(const std::vector<T, Allocator>& value, const WalkOpts& opts,
               U* walker);

template <class U>
void WalkValue(const WalkOpts& opts, std::string* value, U* walker);

template <class T, class Allocator, class U>
void WalkValue(const WalkOpts& opts, std::list<T, Allocator>* value, U* walker);

template <class Key, class Compare, class Allocator, class U>
void WalkValue(const WalkOpts& opts, std::set<Key, Compare, Allocator>* value,
               U* walker);

template <class Key, class T, class Compare, class Allocator, class U>
void WalkValue(const WalkOpts& opts,
               std::map<Key, T, Compare, Allocator>* value, U* walker);

template <class T, class Allocator, class U>
void WalkValue(const WalkOpts& opts, std::vector<T, Allocator>* value,
               U* walker);

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
//    WalkValue Overloads
// -----------------------------------------------------------------------------

template <class U>
void WalkValue(const std::string& value, const WalkOpts& opts, U* walker) {
  walker->ConsumeValue(value);
}

template <class T, class Allocator, class U>
void WalkValue(const std::list<T, Allocator>& value, const WalkOpts& opts,
               U* walker) {
  WalkIterable(value, opts, walker);
}

template <class Key, class Compare, class Allocator, class U>
void WalkValue(const std::set<Key, Compare, Allocator>& value,
               const WalkOpts& opts, U* walker) {
  WalkIterable(value, opts, walker);
}

template <class Key, class T, class Compare, class Allocator, class U>
void WalkValue(const std::map<Key, T, Compare, Allocator>& value,
               const WalkOpts& opts, U* walker) {
  WalkEvent event;
  event.typeno = WalkEvent::LIST_BEGIN;
  walker->ConsumeEvent(event);
  for (auto iter = value.begin(); iter != value.end(); ++iter) {
    event.typeno = WalkEvent::OBJECT_KEY;
    walker->ConsumeEvent(event);
    walker->ConsumeValue(iter->first);
    event.typeno = WalkEvent::VALUE;
    walker->ConsumeEvent(event);
    WalkValue(iter->second, opts, walker);
  }
  event.typeno = WalkEvent::LIST_END;
  walker->ConsumeEvent(event);
}

template <class T, class Allocator, class U>
void WalkValue(const std::vector<T, Allocator>& value, const WalkOpts& opts,
               U* walker) {
  WalkIterable(opts, value, walker);
}

template <class U>
void WalkValue(const WalkOpts& opts, std::string* value, U* walker) {
  walker->ConsumeValue(value);
}

template <class T, class Allocator, class U>
void WalkValue(const WalkOpts& opts, std::list<T, Allocator>* value,
               U* walker) {
  WalkIterable(opts, value, walker);
}

template <class Key, class Compare, class Allocator, class U>
void WalkValue(const WalkOpts& opts, std::set<Key, Compare, Allocator>* value,
               U* walker) {
  WalkIterable(opts, value, walker);
}

template <class Key, class T, class Compare, class Allocator, class U>
void WalkValue(const WalkOpts& opts,
               std::map<Key, T, Compare, Allocator>* value, U* walker) {
  WalkEvent event;
  event.typeno = WalkEvent::LIST_BEGIN;
  walker->ConsumeEvent(event);
  for (auto iter = value->begin(); iter != value->end(); ++iter) {
    event.typeno = WalkEvent::OBJECT_KEY;
    walker->ConsumeEvent(event);
    walker->ConsumeValue(iter->first);
    event.typeno = WalkEvent::VALUE;
    walker->ConsumeEvent(event);
    WalkValue(opts, &iter->second, walker);
  }
  event.typeno = WalkEvent::LIST_END;
  walker->ConsumeEvent(event);
}

template <class T, class Allocator, class U>
void WalkValue(const WalkOpts& opts, std::vector<T, Allocator>* value,
               U* walker) {
  WalkIterable(opts, value, walker);
}

}  // namespace stream
}  // namespace json
