#pragma once
// Copyright 2018 Josh Bialkowski <josh.bialkowski@gmail.com>

#include <cstdio>
#include <string>
#include <vector>

#include <re2/set.h>
#include <re2/stringpiece.h>

#include "tangent/util/fixed_string_stream.h"

#define TANGENT_JSON_VERSION \
  { 0, 2, 6, "dev", 1 }

// Tools for parsing and emitting JSON formatted data
namespace json {

// A reference to a location within the source string
struct SourceLocation {
  uint32_t lineno;  // Number of newlines observed before this point
  uint32_t colno;   // Number of characters since the most recent newline
  uint32_t offset;  // Number of characters since the beginning
};

// Fundamental syntatic unit of a JSON string. Scanner output is a list of
// these
struct Token {
  // Enumerates the possible token types
  enum TypeNo {
    STRING_LITERAL = 0,
    NUMERIC_LITERAL,
    BOOLEAN_LITERAL,
    NULL_LITERAL,
    WHITESPACE,
    PUNCTUATION,
    COMMENT,
  };

  TypeNo typeno;
  re2::StringPiece spelling;
  SourceLocation location;

  static const char* to_string(TypeNo);
};

// Filled for each actionable event discovered by the parser. An actionable
// event is something like an object key, a value literal, or the start of an
// object/list value. Non-actionable events are things like whitespace or
// colon/comma punctuation.
struct Event {
  // Enumerates parse events of interest
  enum TypeNo {
    OBJECT_BEGIN,
    OBJECT_KEY,
    OBJECT_END,
    LIST_BEGIN,
    LIST_END,
    VALUE_LITERAL,
    INVALID,
  };

  TypeNo typeno;  //< What kind of event this is
  Token token;    //< the token for the event

  static const char* to_string(TypeNo value);
};

// Errors are reported via one of these objects
struct Error {
  enum Code {
    NOERROR = 0,
    INTERNAL_ERROR,          //< bug in the code
    LEX_INPUT_FINISHED,      //< lexer has no more input to read from
    LEX_INVALID_TOKEN,       //< lexer encountered invalid json text
    PARSE_UNEXPECTED_TOKEN,  //< valid token but in the wrong place
    PARSE_OOM,               //< Item-parser ran out of item storage
    PARSE_BAD_STATE,         //< Parse failed previously
  };

  // Numeric identifier for the error
  Code code;

  // If the error occured during parse/emit and it was associated with a
  // particular location in the text, this will be set to that location.
  SourceLocation loc;

  // Will be filled with a description of the specific error
  char msg[512];

  // Return the message for the given code
  const char* what() {
    return msg;
  }

  static const char* to_string(Code);
};

// Maintains incremental state for tokenization of a JSON file.
class Scanner {
 public:
  Scanner()
      : piece_(), scanset_(RE2::Options(), RE2::ANCHOR_START), init_state_(0) {}

  // Build and compile the RE2::Set used to scan for patterns. Return value
  // is one of:
  // -1: Error occured
  //  0: Successfully initialized
  //  1: no-op, already successfully initialized
  int init(Error* error = nullptr);

  // Set the contents to be scanned.
  int begin(const re2::StringPiece& piece);

  // Match and return the next token. Return 0 on success and -1 on error.
  // if err is not NULL and an error occurs, will be set to a string
  // describing the error message.
  int pump(Token* tok, Error* error = nullptr) {
    return pump_impl(tok, error, /*peek=*/false);
  }

  // Match and return the next token. Return 0 on succes and -1 on error.
  // if err is not NULL and an error occurs, will be set to a string
  // describing the error message.
  int peek(Token* tok, Error* error = nullptr) {
    return pump_impl(tok, error, /*peek=*/true);
  }

  // Return the current string piece, mostly for debugging purposes
  re2::StringPiece get_piece();

 private:
  int pump_impl(Token* tok, Error* error, bool peek);

  re2::StringPiece piece_;
  RE2::Set scanset_;
  std::vector<int> matches_;

  int init_state_;

  // Total bytes required to store the contents of all numeric values assuming
  // that values are 64-bit aligned.
  size_t numeric_storage_;

  // Total bytes required to store the contents of all string values
  size_t string_storage_;

  // Current location of the input stream
  SourceLocation loc_;
};

// Scan/Tokenize the source string until completion; Store the tokens in `buf`.
// Return the number of tokens that were lexed (which may be greater than `n`)
// or -1 on error.
int lex(const re2::StringPiece& source, Token* buf, size_t n,
        Error* error = nullptr);

// Scan/Tokenize the source string until completion; Store the tokens in `buf`.
// Return the number of tokens that were lexed (which may be greater than `n`)
// or -1 on error.
template <size_t N>
int lex(const re2::StringPiece& source, Token (*buf)[N],
        Error* error = nullptr) {
  return lex(source, &(*buf)[0], N, error);
}

// Lex the entire source and return 0 if no errors are encountered. Return -1
// and fill `error` if any problems are encountered.
int verify_lex(const re2::StringPiece& source, Error* error);

// Manages the state machine for parsing JSON structure from a stream of
// tokens
class Parser {
 public:
  enum State {
    PARSING_VALUE = 0,    // Expect '{', '[' or a value literal
    PARSING_LIST_OPEN,    // Expect a value or a closure
    PARSING_OBJECT_OPEN,  // Expect a key or a closure
    PARSING_KEY,          // Expect a string literal
    PARSING_COLON,        // Expect a ':'
    PARSING_CLOSURE,      // Expect a ']', '}', or ','
    PARSING_ERROR,
  };

  Parser();

  // Reset internal state
  void reset();

  // Advance the internal parse state with the given token. Return 1 if an
  // actionable event has occured.
  /*
   * \return * 1 - an actionable event has occured. `event` is filled
   *         * 0 - no actionable event
   *         * -1 - an error has occured, `error` is filled if not null
   */
  int handle_token(const Token& token, Event* event, Error* error = nullptr,
                   bool dry_run = false);

 protected:
  State state_;

  // TODO(josh): use something fixed-size here
  std::vector<Event::TypeNo> group_stack_;
};

// A combined lexer/parser. Manages the incremental state of both
// simultaniously
class LexerParser {
 public:
  LexerParser() {}
  int init(Error* error = nullptr);
  int begin(const re2::StringPiece& string);

  // Consume tokens until the next semantic event. Return that event in `event`.
  // Advance the token stream past the token that emitted that event.
  int get_next_event(Event* event, Error* error = nullptr);

  // Consume tokens until the next semantic event. Return that event in `event`.
  // Advance the token stream up to but not past the token that generated the
  // `event`. The next call to `get_next_event` will return the same event.
  int peek_next_event(Event* event, Error* error = nullptr);

 private:
  Scanner scanner_;
  Parser parser_;
  Token token_;
};

// Scan/Tokenize and Parse the source string until completion; Store the
// parser events in `buf`. Return the number of events that were parsed (which
// may be greater than `n`)  or -1 on error.
int parse(const re2::StringPiece& source, Event* buf, size_t n,
          Error* error = nullptr);

// Scan/Tokenize and Parse the source string until completion; Store the
// parser events in `buf`. Return the number of events that were parsed (which
// may be greater than `n`)  or -1 on error.
template <size_t N>
int parse(const re2::StringPiece& source, Event (*buf)[N],
          Error* error = nullptr) {
  return parse(source, &(*buf)[0], N, error);
}

// Lex and Parse the entire source and return 0 if no errors are encountered.
// Return -1 and fill `error` if any problems are encountered.
int verify(const re2::StringPiece& source, Error* error = nullptr);

// Options for serialization
struct SerializeOpts {
  size_t indent;          //< Number of spaces to use for indent
  char separators[2][3];  //< map and list separators, i.e. ":" and ","
};

// Safely access elements from a c-style array
template <size_t N>
const char* safe_get(const char* (&map)[N], size_t idx) {
  if (idx < N) {
    return map[idx];
  } else {
    return "<invalid>";
  }
}

// Default serialization option
extern const SerializeOpts kDefaultOpts;

// Serialization options for a very compact JSON output
extern const SerializeOpts kCompactOpts;

// Store an error code and source location and return an ostream that
// can write to the message buffer. The returned stream is value even if
// the error is null (in which case the stream operator is a no-op)
util::FixedBufStream<char> fmt_error(Error* error, Error::Code code,
                                     SourceLocation loc = {});

}  // namespace json
