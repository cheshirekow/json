#pragma once
// Copyright 2018 Josh Bialkowski <josh.bialkowski@gmail.com>

#include "json/json.h"

namespace json {

/// A simple wrapper around Parser::GetNextEvent which allows us to use
/// range-based-for to iterate over events from the parser.
struct EventIterator {
  Event& operator*();
  EventIterator& operator++();

  LexerParser* stream_;
  Event event_;
};

/// A simple layer on top of Parser that allows it to be used in range-for
class Range {
 public:
  explicit Range(LexerParser* stream);
  EventIterator begin();
  EventIterator end();

 private:
  LexerParser* stream_;
};

}  // namespace json

/// Not a general purpose operator. Returns true so long as
/// a.typeno is not one of LIST_END or OBJECT_END. Completely ignores the

bool operator!=(const json::EventIterator& a, const json::EventIterator&);
