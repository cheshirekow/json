#pragma once
// Copyright 2018 Josh Bialkowski <josh.bialkowski@gmail.com>

#include "json/json.h"

namespace json {
namespace stream {

// -----------------------------------------------------------------------------
//    High level API
// -----------------------------------------------------------------------------

// Convenience entry point. Serialize the JSON-stream serializable object
// into the provided character buffer
template <typename T>
void Emit(const T& obj, const SerializeOpts& opts, char* begin, char* end);

// -----------------------------------------------------------------------------
//    Emit Helpers
// -----------------------------------------------------------------------------
// These implement reusable algorithms that are used for multiple different
// types

template <typename T>
void EmitInteger(T value, BufPrinter* out);

template <typename T>
void EmitRealNumber(T value, BufPrinter* out);

void EmitBoolean(bool value, BufPrinter* out);

template <size_t N>
void EmitString(size_t depth, BufPrinter* out);

template <class T, size_t N>
void EmitArray(const T (&arr)[N], const SerializeOpts& opts, size_t depth,
               BufPrinter* out);

// Emit objects from any container that implements the standard iterable
// concept (i.e. `.begin()` and `.end()` return iterator of some type that
// deferences to `::value_type`.
template <typename T>
void EmitIterable(const T& obj, const SerializeOpts& opts, size_t depth,
                  BufPrinter* out);

// Template to emit a field in an object map
template <typename T>
void EmitField(const char* key, const T& value, const SerializeOpts& opts,
               size_t depth, BufPrinter* out);

// Emit a separator between two consecutive fields. This generally includes a
// comma and possibly a newline.
void EmitFieldSep(const SerializeOpts& opts, BufPrinter* out);

// -----------------------------------------------------------------------------
//    EmitValue Overloads
// -----------------------------------------------------------------------------

void EmitValue(int8_t value, const SerializeOpts& opts, size_t depth,
               BufPrinter* out);
void EmitValue(int16_t value, const SerializeOpts& opts, size_t depth,
               BufPrinter* out);
void EmitValue(int32_t value, const SerializeOpts& opts, size_t depth,
               BufPrinter* out);
void EmitValue(int64_t value, const SerializeOpts& opts, size_t depth,
               BufPrinter* out);
void EmitValue(uint8_t value, const SerializeOpts& opts, size_t depth,
               BufPrinter* out);
void EmitValue(uint16_t value, const SerializeOpts& opts, size_t depth,
               BufPrinter* out);
void EmitValue(uint32_t value, const SerializeOpts& opts, size_t depth,
               BufPrinter* out);
void EmitValue(uint64_t value, const SerializeOpts& opts, size_t depth,
               BufPrinter* out);
void EmitValue(float value, const SerializeOpts& opts, size_t depth,
               BufPrinter* out);
void EmitValue(double value, const SerializeOpts& opts, size_t depth,
               BufPrinter* out);
void EmitValue(bool value, const SerializeOpts& opts, size_t depth,
               BufPrinter* out);

template <size_t N>
void EmitValue(const char (&value)[N], const SerializeOpts& opts, size_t depth,
               BufPrinter* out);

template <typename T, size_t N>
void EmitValue(const T (&arr)[N], const SerializeOpts& opts, size_t depth,
               BufPrinter* out);

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
//    Emitter helpers
// -----------------------------------------------------------------------------

template <typename T>
void EmitInteger(T value, BufPrinter* out) {
  (*out)("%ld", static_cast<long>(value));  // NOLINT(runtime/int)
}

template <typename T>
void EmitRealNumber(T value, BufPrinter* out) {
  (*out)("%f", static_cast<double>(value));
}

template <size_t N>
void EmitString(const char (&arr)[N], BufPrinter* out) {
  (*out)("\"%s\"", arr);
}

template <typename T, size_t N>
void EmitValue(const T (&arr)[N], const SerializeOpts& opts, size_t depth,
               BufPrinter* out) {
  EmitArray(arr, opts, depth, out);
}

// -----------------------------------------------------------------------------
//    Emitter Overloads
// -----------------------------------------------------------------------------

template <size_t N>
void EmitValue(const char (&value)[N], const SerializeOpts& opts, size_t depth,
               BufPrinter* out) {
  EmitString(value, out);
}

}  // namespace stream
}  // namespace json
