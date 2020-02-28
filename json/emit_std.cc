// Copyright 2018 Josh Bialkowski <josh.bialkowski@gmail.com>
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <glog/logging.h>

#include "json/emit_std.h"

namespace json {
namespace stream {

// -----------------------------------------------------------------------------
//    Emitter Helpers
// -----------------------------------------------------------------------------

void EmitString(const std::string& str, BufPrinter* out) {
  (*out)("\"%s\"", str.c_str());
}

// -----------------------------------------------------------------------------
//    Emitter Overloads
// -----------------------------------------------------------------------------

void EmitValue(const std::string& value, const SerializeOpts& opts,
               size_t depth, BufPrinter* out) {
  EmitString(value, out);
}

}  // namespace stream
}  // namespace json
