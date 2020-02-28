#pragma once
// Copyright 2018 Josh Bialkowski <josh.bialkowski@gmail.com>

#include <fmt/format.h>
#include <fmt/ostream.h>
#include <glog/logging.h>

#include "json/emit.h"
#include "json/json.h"
#include "json/parse.h"

namespace json {

// Streaming API for creating json-serializable structures
namespace stream {

// -----------------------------------------------------------------------------
//    High level API
// -----------------------------------------------------------------------------

// Convenience entry point. Walk the JSON-stream serializable object calling
// callback at every step. Callback should be an object with one method of
// the form:
//
//  void ConsumeEvent(const WalkEvent& event);
//
// and multiple methods
//
//  void ConsumeValue(const T& value);
//
// for every value type `T` that is expected to encounter. ConsumeValue may
// be a template, depending on the implementation of the walk.
template <typename T, typename U>
void Walk(const T& obj, const WalkOpts& opts, U* walker);

template <typename T, typename U>
void Walk(const WalkOpts& opts, T* obj, U* walker);

// -----------------------------------------------------------------------------
//    Walk Helpers
// -----------------------------------------------------------------------------

// Walk objects from any container that implements the standard iterable
// concept (i.e. `.begin()` and `.end()` return iterator of some type that
// deferences to `::value_type`.
template <class T, class U>
void WalkIterable(const WalkOpts& opts, T* obj, U* walker);

// -----------------------------------------------------------------------------
//    WalkValue Overloads
// -----------------------------------------------------------------------------

template <class T>
void WalkValue(int8_t value, const WalkOpts& opts, T* walker);

template <class T>
void WalkValue(int16_t value, const WalkOpts& opts, T* walker);

template <class T>
void WalkValue(int32_t value, const WalkOpts& opts, T* walker);

template <class T>
void WalkValue(int64_t value, const WalkOpts& opts, T* walker);

template <class T>
void WalkValue(uint8_t value, const WalkOpts& opts, T* walker);

template <class T>
void WalkValue(uint16_t value, const WalkOpts& opts, T* walker);

template <class T>
void WalkValue(uint32_t value, const WalkOpts& opts, T* walker);

template <class T>
void WalkValue(uint64_t value, const WalkOpts& opts, T* walker);

template <class T>
void WalkValue(float value, const WalkOpts& opts, T* walker);

template <class T>
void WalkValue(double value, const WalkOpts& opts, T* walker);

template <class T>
void WalkValue(bool value, const WalkOpts& opts, T* walker);

template <size_t N, class T>
void WalkValue(const char (&value)[N], const WalkOpts& opts, T* walker);

template <typename T, size_t N, class U>
void WalkValue(const T (&value)[N], const WalkOpts& opts, U* walker);

template <class T>
void WalkValue(const WalkOpts& opts, int8_t* value, T* walker);

template <class T>
void WalkValue(const WalkOpts& opts, int16_t* value, T* walker);

template <class T>
void WalkValue(const WalkOpts& opts, int32_t* value, T* walker);

template <class T>
void WalkValue(const WalkOpts& opts, int64_t* value, T* walker);

template <class T>
void WalkValue(const WalkOpts& opts, uint8_t* value, T* walker);

template <class T>
void WalkValue(const WalkOpts& opts, uint16_t* value, T* walker);

template <class T>
void WalkValue(const WalkOpts& opts, uint32_t* value, T* walker);

template <class T>
void WalkValue(const WalkOpts& opts, uint64_t* value, T* walker);

template <class T>
void WalkValue(const WalkOpts& opts, float* value, T* walker);

template <class T>
void WalkValue(const WalkOpts& opts, double* value, T* walker);

template <class T>
void WalkValue(const WalkOpts& opts, bool* value, T* walker);

template <size_t N, class T>
void WalkValue(const WalkOpts& opts, char (*value)[N], T* walker);

template <typename T, size_t N, class U>
void WalkValue(const WalkOpts& opts, T (*value)[N], U* walker);

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
//    WalkValue Helpers
// -----------------------------------------------------------------------------

template <class T, class U>
void WalkIterable(const WalkOpts& opts, T* obj, U* walker) {
  WalkEvent event;
  event.typeno = WalkEvent::LIST_BEGIN;
  walker->ConsumeEvent(event);

  event.typeno = WalkEvent::VALUE;
  for (auto iter = obj->begin(); iter != obj->end(); ++iter) {
    walker->ConsumeEvent(event);
    WalkValue(*iter, opts, walker);
  }

  event.typeno = WalkEvent::LIST_END;
  walker->ConsumeEvent(event);
}

// -----------------------------------------------------------------------------
//    WalkValue Templates
// -----------------------------------------------------------------------------

template <class T>
void WalkValue(int8_t value, const WalkOpts& opts, T* walker) {
  walker->ConsumeValue(value);
}

template <class T>
void WalkValue(int16_t value, const WalkOpts& opts, T* walker) {
  walker->ConsumeValue(value);
}

template <class T>
void WalkValue(int32_t value, const WalkOpts& opts, T* walker) {
  walker->ConsumeValue(value);
}

template <class T>
void WalkValue(int64_t value, const WalkOpts& opts, T* walker) {
  walker->ConsumeValue(value);
}

template <class T>
void WalkValue(uint8_t value, const WalkOpts& opts, T* walker) {
  walker->ConsumeValue(value);
}

template <class T>
void WalkValue(uint16_t value, const WalkOpts& opts, T* walker) {
  walker->ConsumeValue(value);
}

template <class T>
void WalkValue(uint32_t value, const WalkOpts& opts, T* walker) {
  walker->ConsumeValue(value);
}

template <class T>
void WalkValue(uint64_t value, const WalkOpts& opts, T* walker) {
  walker->ConsumeValue(value);
}

template <class T>
void WalkValue(float value, const WalkOpts& opts, T* walker) {
  walker->ConsumeValue(value);
}

template <class T>
void WalkValue(double value, const WalkOpts& opts, T* walker) {
  walker->ConsumeValue(value);
}

template <class T>
void WalkValue(bool value, const WalkOpts& opts, T* walker) {
  walker->ConsumeValue(value);
}

template <size_t N, class T>
void WalkValue(const char (&value)[N], const WalkOpts& opts, T* walker) {
  WalkEvent event;
  event.typeno = WalkEvent::LIST_BEGIN;
  walker->ConsumeEvent(event);
  for (size_t idx = 0; idx < N; idx++) {
    WalkValue(value[idx], opts, walker);
  }
  event.typeno = WalkEvent::LIST_END;
  walker->ConsumeEvent(event);
}

template <typename T, size_t N, class U>
void WalkValue(const T (&value)[N], const WalkOpts& opts, U* walker) {
  WalkEvent event;
  event.typeno = WalkEvent::LIST_BEGIN;
  walker->ConsumeEvent(event);
  for (size_t idx = 0; idx < N; idx++) {
    WalkValue(value[idx], opts, walker);
  }
  event.typeno = WalkEvent::LIST_END;
  walker->ConsumeEvent(event);
}

template <class T>
void WalkValue(const WalkOpts& opts, int8_t* value, T* walker) {
  walker->ConsumeValue(value);
}

template <class T>
void WalkValue(const WalkOpts& opts, int16_t* value, T* walker) {
  walker->ConsumeValue(value);
}

template <class T>
void WalkValue(const WalkOpts& opts, int32_t* value, T* walker) {
  walker->ConsumeValue(value);
}

template <class T>
void WalkValue(const WalkOpts& opts, int64_t* value, T* walker) {
  walker->ConsumeValue(value);
}

template <class T>
void WalkValue(const WalkOpts& opts, uint8_t* value, T* walker) {
  walker->ConsumeValue(value);
}

template <class T>
void WalkValue(const WalkOpts& opts, uint16_t* value, T* walker) {
  walker->ConsumeValue(value);
}

template <class T>
void WalkValue(const WalkOpts& opts, uint32_t* value, T* walker) {
  walker->ConsumeValue(value);
}

template <class T>
void WalkValue(const WalkOpts& opts, uint64_t* value, T* walker) {
  walker->ConsumeValue(value);
}

template <class T>
void WalkValue(const WalkOpts& opts, float* value, T* walker) {
  walker->ConsumeValue(value);
}

template <class T>
void WalkValue(const WalkOpts& opts, double* value, T* walker) {
  walker->ConsumeValue(value);
}

template <class T>
void WalkValue(const WalkOpts& opts, bool* value, T* walker) {
  walker->ConsumeValue(value);
}

template <size_t N, class T>
void WalkValue(const WalkOpts& opts, char (*value)[N], T* walker) {
  WalkEvent event;
  event.typeno = WalkEvent::LIST_BEGIN;
  walker->ConsumeEvent(event);
  for (size_t idx = 0; idx < N; idx++) {
    WalkValue((*value)[idx], opts, walker);
  }
  event.typeno = WalkEvent::LIST_END;
  walker->ConsumeEvent(event);
}

template <typename T, size_t N, class U>
void WalkValue(const WalkOpts& opts, T (*value)[N], U* walker) {
  WalkEvent event;
  event.typeno = WalkEvent::LIST_BEGIN;
  walker->ConsumeEvent(event);
  for (size_t idx = 0; idx < N; idx++) {
    WalkValue((*value)[idx], opts, walker);
  }
  event.typeno = WalkEvent::LIST_END;
  walker->ConsumeEvent(event);
}

}  // namespace stream
}  // namespace json
