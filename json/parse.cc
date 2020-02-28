// Copyright 2018 Josh Bialkowski <josh.bialkowski@gmail.com>
#include "json/parse.h"

#include <fmt/format.h>
#include <fmt/ostream.h>
#include <glog/logging.h>

namespace json {

// -----------------------------------------------------------------------------
//    Parser Implementations
// -----------------------------------------------------------------------------

int ParseBoolean(const Token& token, bool* value) {
  switch (token.typeno) {
    case json::Token::BOOLEAN_LITERAL: {
      if (token.spelling == "true") {
        (*value) = true;
        return 0;
      } else if (token.spelling == "false") {
        (*value) = false;
        return 0;
      } else {
        // Lexer shouldn't match a BOOLEAN_LITERAL for any other strings.
        LOG(ERROR) << "I should never be here!";
      }
      break;
    }

    case json::Token::STRING_LITERAL: {
      LOG(WARNING) << fmt::format("Parsing string literal as boolean: {}",
                                  token.spelling);
      if (RE2::FullMatch(token.spelling, "(?i)y(es)?|t(rue)?|on|1")) {
        (*value) = true;
        return 0;
      } else if (RE2::FullMatch(token.spelling, "(?i)no?|f(alse)?|off|0")) {
        (*value) = false;
        return 0;
      } else {
        LOG(WARNING) << fmt::format(
            "Ambiguous truthiness of string '{}' can't be parsed as a boolean. "
            "Assuming 'true'",
            token.spelling);
        (*value) = true;
      }
    }

    default:
      LOG(WARNING) << fmt::format("Can't parse {}({}) as boolean",
                                  token.spelling,
                                  json::Token::ToString(token.typeno));
  }
  return 1;
}

// -----------------------------------------------------------------------------
//    Sink Functions
// -----------------------------------------------------------------------------

int SinkValue(LexerParser* stream) {
  json::Event event;
  json::Error error;
  if (stream->GetNextEvent(&event, &error)) {
    LOG(WARNING) << fmt::format("In {}, Failed to get JSON scalar event",
                                __PRETTY_FUNCTION__);
    return 1;
  }

  return SinkValue(event, stream);
}

int SinkValue(const Event& event, LexerParser* stream) {
  switch (event.typeno) {
    case json::Event::OBJECT_BEGIN:
      return SinkObject(stream);
    case json::Event::LIST_BEGIN:
      return SinkList(stream);
    case json::Event::VALUE_LITERAL:
      return 0;
    default:
      LOG(WARNING) << fmt::format(
          "{}:{} Unexpected {} event at {}:{}", __PRETTY_FUNCTION__, __LINE__,
          json::Event::ToString(event.typeno), event.token.location.lineno,
          event.token.location.colno);
  }
  return 1;
}

int SinkObject(LexerParser* stream) {
  json::Event event;
  json::Error error;

  if (stream->GetNextEvent(&event, &error) != 0) {
    LOG(WARNING) << fmt::format(
        "Expected a JSON object but failed to get the next event, {} at {}:{}",
        error.msg, __PRETTY_FUNCTION__, __LINE__);
    return 1;
  }

  if (event.typeno != Event::OBJECT_BEGIN) {
    LOG(WARNING) << fmt::format(
        "{}:{} Unexpected {} event at {}:{}", __PRETTY_FUNCTION__, __LINE__,
        json::Event::ToString(event.typeno), event.token.location.lineno,
        event.token.location.colno);
    return 1;
  }

  uint32_t object_count = 1;
  while (stream->GetNextEvent(&event, &error) != 0) {
    switch (event.typeno) {
      case json::Event::OBJECT_BEGIN:
        ++object_count;
        break;
      case json::Event::OBJECT_END:
        if (--object_count == 0) {
          return 0;
        }
        break;
      default:
        break;
    }
  }

  LOG(WARNING) << error.msg
               << fmt::format(" at {}:{}", __PRETTY_FUNCTION__, __LINE__);
  return 1;
}

int SinkList(LexerParser* stream) {
  json::Event event;
  json::Error error;

  if (stream->GetNextEvent(&event, &error) != 0) {
    LOG(WARNING) << fmt::format(
        "Expected a JSON list but failed to get the next event, {} at {}:{}",
        error.msg, __PRETTY_FUNCTION__, __LINE__);
    return 1;
  }

  if (event.typeno != Event::LIST_BEGIN) {
    LOG(WARNING) << fmt::format(
        "{}:{} Unexpected {} event at {}:{}", __PRETTY_FUNCTION__, __LINE__,
        json::Event::ToString(event.typeno), event.token.location.lineno,
        event.token.location.colno);
    return 1;
  }

  uint32_t list_count = 1;
  while (stream->GetNextEvent(&event, &error) != 0) {
    switch (event.typeno) {
      case json::Event::LIST_BEGIN:
        ++list_count;
        break;
      case json::Event::LIST_END:
        if (--list_count == 0) {
          return 0;
        }
        break;
      default:
        break;
    }
  }

  LOG(WARNING) << error.msg
               << fmt::format(" at {}:{}", __PRETTY_FUNCTION__, __LINE__);
  return 1;
}

}  // namespace json
