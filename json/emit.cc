// Copyright 2018 Josh Bialkowski <josh.bialkowski@gmail.com>
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <glog/logging.h>

#include "json/emit.h"

namespace json {
namespace stream {

// -----------------------------------------------------------------------------
//    Emitter Implementations
// -----------------------------------------------------------------------------

void EmitBoolean(bool value, BufPrinter* out) {
  if (value) {
    (*out)("true");
  } else {
    (*out)("false");
  }
}

void EmitFieldSep(const SerializeOpts& opts, BufPrinter* out) {
  (*out)("%s", opts.separators[1]);
  if (opts.indent) {
    (*out)("\n");
  }
}

// -----------------------------------------------------------------------------
//    Emitter Overloads
// -----------------------------------------------------------------------------

void EmitValue(int8_t value, const SerializeOpts& opts, size_t depth,
               BufPrinter* out) {
  EmitInteger(value, out);
}

void EmitValue(int16_t value, const SerializeOpts& opts, size_t depth,
               BufPrinter* out) {
  EmitInteger(value, out);
}

void EmitValue(int32_t value, const SerializeOpts& opts, size_t depth,
               BufPrinter* out) {
  EmitInteger(value, out);
}

void EmitValue(int64_t value, const SerializeOpts& opts, size_t depth,
               BufPrinter* out) {
  EmitInteger(value, out);
}

void EmitValue(uint8_t value, const SerializeOpts& opts, size_t depth,
               BufPrinter* out) {
  EmitInteger(value, out);
}

void EmitValue(uint16_t value, const SerializeOpts& opts, size_t depth,
               BufPrinter* out) {
  EmitInteger(value, out);
}

void EmitValue(uint32_t value, const SerializeOpts& opts, size_t depth,
               BufPrinter* out) {
  EmitInteger(value, out);
}

void EmitValue(uint64_t value, const SerializeOpts& opts, size_t depth,
               BufPrinter* out) {
  EmitInteger(value, out);
}

void EmitValue(float value, const SerializeOpts& opts, size_t depth,
               BufPrinter* out) {
  EmitRealNumber(value, out);
}

void EmitValue(double value, const SerializeOpts& opts, size_t depth,
               BufPrinter* out) {
  EmitRealNumber(value, out);
}

void EmitValue(bool value, const SerializeOpts& opts, size_t depth,
               BufPrinter* out) {
  EmitBoolean(value, out);
}

}  // namespace stream
}  // namespace json
