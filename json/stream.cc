// Copyright 2018 Josh Bialkowski <josh.bialkowski@gmail.com>
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <glog/logging.h>

#include "json/stream.h"
#include "json/stream_tpl.h"

namespace json {
namespace stream {

// -----------------------------------------------------------------------------
//    Sink Functions
// -----------------------------------------------------------------------------

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
//    Parse Overloads
// -----------------------------------------------------------------------------
void ParseValue(const Event& event, LexerParser* stream, int8_t* value) {
  // TODO(josh): move to common location
  if (event.typeno != json::Event::VALUE_LITERAL) {
    LOG(WARNING) << fmt::format("Cannot parse {} as value at {}:{}",
                                json::Event::ToString(event.typeno),
                                event.token.location.lineno,
                                event.token.location.colno);
  }
  ParseInteger(event.token, value);
}

void ParseValue(const Event& event, LexerParser* stream, int16_t* value) {
  // TODO(josh): move to common location
  if (event.typeno != json::Event::VALUE_LITERAL) {
    LOG(WARNING) << fmt::format("Cannot parse {} as value at {}:{}",
                                json::Event::ToString(event.typeno),
                                event.token.location.lineno,
                                event.token.location.colno);
  }
  ParseInteger(event.token, value);
}

void ParseValue(const Event& event, LexerParser* stream, int32_t* value) {
  // TODO(josh): move to common location
  if (event.typeno != json::Event::VALUE_LITERAL) {
    LOG(WARNING) << fmt::format("Cannot parse {} as value at {}:{}",
                                json::Event::ToString(event.typeno),
                                event.token.location.lineno,
                                event.token.location.colno);
  }
  ParseInteger(event.token, value);
}

void ParseValue(const Event& event, LexerParser* stream, int64_t* value) {
  // TODO(josh): move to common location
  if (event.typeno != json::Event::VALUE_LITERAL) {
    LOG(WARNING) << fmt::format("Cannot parse {} as value at {}:{}",
                                json::Event::ToString(event.typeno),
                                event.token.location.lineno,
                                event.token.location.colno);
  }
  ParseInteger(event.token, value);
}

void ParseValue(const Event& event, LexerParser* stream, uint8_t* value) {
  // TODO(josh): move to common location
  if (event.typeno != json::Event::VALUE_LITERAL) {
    LOG(WARNING) << fmt::format("Cannot parse {} as value at {}:{}",
                                json::Event::ToString(event.typeno),
                                event.token.location.lineno,
                                event.token.location.colno);
  }
  ParseInteger(event.token, value);
}

void ParseValue(const Event& event, LexerParser* stream, uint16_t* value) {
  // TODO(josh): move to common location
  if (event.typeno != json::Event::VALUE_LITERAL) {
    LOG(WARNING) << fmt::format("Cannot parse {} as value at {}:{}",
                                json::Event::ToString(event.typeno),
                                event.token.location.lineno,
                                event.token.location.colno);
  }
  ParseInteger(event.token, value);
}

void ParseValue(const Event& event, LexerParser* stream, uint32_t* value) {
  // TODO(josh): move to common location
  if (event.typeno != json::Event::VALUE_LITERAL) {
    LOG(WARNING) << fmt::format("Cannot parse {} as value at {}:{}",
                                json::Event::ToString(event.typeno),
                                event.token.location.lineno,
                                event.token.location.colno);
  }
  ParseInteger(event.token, value);
}

void ParseValue(const Event& event, LexerParser* stream, uint64_t* value) {
  // TODO(josh): move to common location
  if (event.typeno != json::Event::VALUE_LITERAL) {
    LOG(WARNING) << fmt::format("Cannot parse {} as value at {}:{}",
                                json::Event::ToString(event.typeno),
                                event.token.location.lineno,
                                event.token.location.colno);
  }
  ParseInteger(event.token, value);
}

void ParseValue(const Event& event, LexerParser* stream, double* value) {
  // TODO(josh): move to common location
  if (event.typeno != json::Event::VALUE_LITERAL) {
    LOG(WARNING) << fmt::format("Cannot parse {} as value at {}:{}",
                                json::Event::ToString(event.typeno),
                                event.token.location.lineno,
                                event.token.location.colno);
  }
  ParseRealNumber(event.token, value);
}

void ParseValue(const Event& event, LexerParser* stream, float* value) {
  // TODO(josh): move to common location
  if (event.typeno != json::Event::VALUE_LITERAL) {
    LOG(WARNING) << fmt::format("Cannot parse {} as value at {}:{}",
                                json::Event::ToString(event.typeno),
                                event.token.location.lineno,
                                event.token.location.colno);
  }
  ParseRealNumber(event.token, value);
}

void ParseValue(const Event& event, LexerParser* stream, bool* value) {
  // TODO(josh): move to common location
  if (event.typeno != json::Event::VALUE_LITERAL) {
    LOG(WARNING) << fmt::format("Cannot parse {} as value at {}:{}",
                                json::Event::ToString(event.typeno),
                                event.token.location.lineno,
                                event.token.location.colno);
  }
  ParseBoolean(event.token, value);
}

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
