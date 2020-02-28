// Copyright 2018 Josh Bialkowski <josh.bialkowski@gmail.com>

#include "json/pipeline.h"

namespace json {

// -----------------------------------------------------------------------------
//    EventIterator
// -----------------------------------------------------------------------------

Event& EventIterator::operator*() {
  return event_;
}

EventIterator& EventIterator::operator++() {
  stream_->get_next_event(&event_);
  return *this;
}

// -----------------------------------------------------------------------------
//    Range
// -----------------------------------------------------------------------------

Range::Range(LexerParser* stream) : stream_(stream) {}

EventIterator Range::begin() {
  return EventIterator{stream_};
}

// The iterator returned from this is not actually used because the quality
// comparator always returns false.
EventIterator Range::end() {
  return {stream_, {Event::INVALID, {}}};
}

}  // namespace json

bool operator!=(const json::EventIterator& a, const json::EventIterator&) {
  if (a.event_.typeno == json::Event::OBJECT_END ||
      a.event_.typeno == json::Event::LIST_END) {
    return false;
  }
  return true;
}
