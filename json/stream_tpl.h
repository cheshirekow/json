#pragma once
// Copyright 2018 Josh Bialkowski <josh.bialkowski@gmail.com>

#include "json/stream.h"

namespace json {
namespace stream {

// -----------------------------------------------------------------------------
//    High Level
// -----------------------------------------------------------------------------

template <typename T, typename U>
void Walk(const T& obj, const WalkOpts& opts, U* walker) {
  WalkValue(obj, opts, walker);
}

template <typename T, typename U>
void Walk(const WalkOpts& opts, T* obj, U* walker) {
  WalkValue(opts, obj, walker);
}

// NOTE(josh): ideally we would get these definitions out of this header file,
// however they must come *AFTER* all declarations for the internal functions
// ParseValue (etc)... otherwise name lookup will fail.

template <typename T>
void Parse(json::LexerParser* stream, T* out) {
  Event event;
  Error error;
  if (stream->GetNextEvent(&event, &error) != 0) {
    LOG(ERROR) << "Invalid JSON: " << error.msg;
    return;
  }
  ParseValue(event, stream, out);
}

template <typename T>
void Parse(const re2::StringPiece& content, T* out) {
  json::LexerParser stream{};
  json::Error error;
  CHECK_EQ(0, stream.Init(&error)) << error.msg;
  CHECK_EQ(0, stream.Begin(content));
  Parse(&stream, out);
}

template <typename T>
void Emit(const T& obj, const SerializeOpts& opts, char* begin, char* end) {
  BufPrinter out{nullptr, nullptr};
  EmitValue(obj, opts, 0, &out);
}

// TODO(josh): get this back out of the non-std header
template <typename T>
std::string Emit(const T& obj, const SerializeOpts& opts) {
  BufPrinter out1{nullptr, nullptr};
  EmitValue(obj, opts, 0, &out1);
  std::string buf;
  buf.resize(out1.Size() + 1, '\0');
  BufPrinter out2{&buf[0], &buf[buf.size()]};
  EmitValue(obj, opts, 0, &out2);
  buf.resize(out2.Size());
  return buf;
}

// -----------------------------------------------------------------------------
//    Parser Helpers
// -----------------------------------------------------------------------------

template <class T, size_t N>
void ParseArray(LexerParser* stream, T (*arr)[N]) {
  T* iter = &(*arr)[0];
  Event event{};
  while (stream->GetNextEvent(&event) == 0) {
    switch (event.typeno) {
      case Event::LIST_BEGIN:
        break;

      case Event::LIST_END:
        return;

      default:
        if (iter < &(*arr)[N]) {
          ParseValue(event, stream, iter);
          iter++;
        } else {
          printf("WARNING: skipping elements after %d", static_cast<int>(N));
        }
        break;
    }
  }
}

template <typename T>
void ParseObject(const Event& entry_event, LexerParser* stream, T* out) {
  if (entry_event.typeno != json::Event::OBJECT_BEGIN) {
    LOG(WARNING) << fmt::format(
        "Expected JSON object for {}, but instead got {} at {}:{}",
        __PRETTY_FUNCTION__, json::Event::ToString(entry_event.typeno),
        entry_event.token.location.lineno, entry_event.token.location.colno);
    SinkValue(entry_event, stream);
    return;
  }

  json::Event event;
  json::Error error;
  while (stream->GetNextEvent(&event, &error) == 0) {
    if (event.typeno == json::Event::OBJECT_END) {
      return;
    }

    if (event.typeno != json::Event::OBJECT_KEY) {
      LOG(WARNING) << fmt::format(
          "{}:{} Unexpected {} event at {}:{}", __PRETTY_FUNCTION__, __LINE__,
          json::Event::ToString(event.typeno), event.token.location.lineno,
          event.token.location.colno);
      return;
    }

    // event.typeno == OBJECT_KEY, as expected
    Token keytoken = event.token;

    // NOTE(josh): strip literal quotes off of string token
    re2::StringPiece keyvalue =
        keytoken.spelling.substr(1, keytoken.spelling.size() - 2);

    if (stream->GetNextEvent(&event, &error) != 0) {
      LOG(WARNING) << error.msg;
      return;
    }

    if (ParseField(keyvalue, event, stream, out)) {
      LOG(WARNING) << fmt::format(
          "Unrecognized key {}({}) at {}:{}", keyvalue, RuntimeHash(keyvalue),
          keytoken.location.lineno, keytoken.location.colno);
    }
  }
  LOG(WARNING) << error.msg;
}

template <typename T>
void ParseInsertable(const Event& entry_event, LexerParser* stream, T* out) {
  if (entry_event.typeno != json::Event::LIST_BEGIN) {
    LOG(WARNING) << fmt::format("Can't parse {} as a list at {}:{}",
                                json::Event::ToString(entry_event.typeno),
                                entry_event.token.location.lineno,
                                entry_event.token.location.colno);
    SinkValue(entry_event, stream);
    return;
  }

  // TODO(josh): should we?
  // out->clear();
  std::insert_iterator<T> iter(*out, out->begin());

  json::Event event{};
  json::Error error{};
  while (stream->GetNextEvent(&event, &error) == 0) {
    if (event.typeno == json::Event::LIST_END) {
      return;
    }
    typename T::value_type value{};
    ParseValue(event, stream, &value);
    *(iter++) = value;
  }
  LOG(WARNING) << error.msg;
}

// -----------------------------------------------------------------------------
//    Emit Helpers
// -----------------------------------------------------------------------------

template <typename T>
void EmitField(const char* key, const T& value, const SerializeOpts& opts,
               size_t depth, BufPrinter* out) {
  FmtIndent(out, opts.indent, depth + 1);
  (*out)("\"%s\"%s", key, opts.separators[0]);
  EmitValue(value, opts, depth + 1, out);
}

template <class T, size_t N>
void EmitArray(const T (&arr)[N], const SerializeOpts& opts, size_t depth,
               BufPrinter* out) {
  (*out)("[");
  if (opts.indent) {
    (*out)("\n");
  }
  const T* iter = &arr[0];
  while (iter < &arr[N]) {
    FmtIndent(out, opts.indent, depth + 1);
    EmitValue(*iter, opts, depth + 1, out);
    ++iter;
    if (iter < &arr[N]) {
      (*out)("%s", opts.separators[1]);
    }
    if (opts.indent) {
      (*out)("\n");
    }
  }
  (*out)("]");
}

template <typename T>
void EmitIterable(const T& obj, const SerializeOpts& opts, size_t depth,
                  BufPrinter* out) {
  if (obj.size() < 1) {
    (*out)("[]");
  } else {
    (*out)("[");
    if (opts.indent) {
      (*out)("\n");
    }
    auto iter = obj.begin();
    while (iter != obj.end()) {
      FmtIndent(out, opts.indent, depth + 1);
      EmitValue(*iter, opts, depth + 1, out);
      ++iter;
      if (iter != obj.end()) {
        (*out)("%s", opts.separators[1]);
      }
      if (opts.indent) {
        (*out)("\n");
      }
    }
    FmtIndent(out, opts.indent, depth);
    (*out)("]");
  }
}

}  // namespace stream
}  // namespace json
