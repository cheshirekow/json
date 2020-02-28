// Copyright 2018 Josh Bialkowski <josh.bialkowski@gmail.com>
#include "json/parse.h"

#include <fmt/format.h>
#include <fmt/ostream.h>
#include <glog/logging.h>

namespace json {

// -----------------------------------------------------------------------------
//    Parser Implementations
// -----------------------------------------------------------------------------

void ParseBoolean(const Token& token, bool* value) {
  switch (token.typeno) {
    case json::Token::BOOLEAN_LITERAL: {
      if (token.spelling == "true") {
        (*value) = true;
      } else if (token.spelling == "false") {
        (*value) = false;
      } else {
        LOG(ERROR) << "I should never be here!";
      }
      break;
    }

    case json::Token::STRING_LITERAL: {
      LOG(WARNING) << fmt::format("Parsing string literal as boolean: {}",
                                  token.spelling);
      if (RE2::FullMatch(token.spelling, "(?i)y(es)?|t(rue)?|on|1")) {
        (*value) = true;
      } else if (RE2::FullMatch(token.spelling, "(?i)no?|f(alse)?|off|0")) {
        (*value) = false;
      } else {
        LOG(WARNING) << fmt::format("Can't parse {} as boolean",
                                    token.spelling);
      }
    }

    default:
      LOG(WARNING) << fmt::format("Can't parse {}({}) as boolean",
                                  token.spelling,
                                  json::Token::ToString(token.typeno));
  }
}

// -----------------------------------------------------------------------------
//    Sink Functions
// -----------------------------------------------------------------------------

void SinkValue(LexerParser* stream) {
  json::Event event;
  json::Error error;
  if (stream->GetNextEvent(&event, &error)) {
    LOG(WARNING) << fmt::format("In {}, Failed to get JSON scalar event",
                                __PRETTY_FUNCTION__);
    return;
  }

  switch (event.typeno) {
    case json::Event::OBJECT_BEGIN:
      SinkObject(stream);
    case json::Event::LIST_BEGIN:
      SinkList(stream);
    case json::Event::VALUE_LITERAL:
      break;
    default:
      LOG(WARNING) << fmt::format(
          "{}:{} Unexpected {} event at {}:{}", __PRETTY_FUNCTION__, __LINE__,
          json::Event::ToString(event.typeno), event.token.location.lineno,
          event.token.location.colno);
  }
}

void SinkValue(const Event& event, LexerParser* stream) {
  switch (event.typeno) {
    case json::Event::OBJECT_BEGIN:
      SinkObject(stream);
    case json::Event::LIST_BEGIN:
      SinkList(stream);
    case json::Event::VALUE_LITERAL:
      break;
    default:
      LOG(WARNING) << fmt::format(
          "{}:{} Unexpected {} event at {}:{}", __PRETTY_FUNCTION__, __LINE__,
          json::Event::ToString(event.typeno), event.token.location.lineno,
          event.token.location.colno);
  }
}

void SinkObject(LexerParser* stream) {
  json::Event event;
  json::Error error;
  while (true) {
    if (stream->GetNextEvent(&event, &error) != 0) {
      break;
    }
    switch (event.typeno) {
      case json::Event::OBJECT_KEY:
        if (stream->GetNextEvent(&event, &error) != 0) {
          break;
        }
        SinkValue(event, stream);
        break;
      case json::Event::OBJECT_END:
        return;
      default:
        LOG(WARNING) << fmt::format(
            "{}:{} Unexpected {} event at {}:{}", __PRETTY_FUNCTION__, __LINE__,
            json::Event::ToString(event.typeno), event.token.location.lineno,
            event.token.location.colno);
        return;
    }
  }
  LOG(WARNING) << error.msg;
}

void SinkList(LexerParser* stream) {
  json::Event event;
  json::Error error;
  while (true) {
    if (stream->GetNextEvent(&event, &error) != 0) {
      break;
    }
    switch (event.typeno) {
      case json::Event::LIST_END:
        return;
      default:
        SinkValue(event, stream);
    }
  }
  LOG(WARNING) << error.msg;
}

}  // namespace json
