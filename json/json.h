#pragma once
// Copyright 2018 Josh Bialkowski <josh.bialkowski@gmail.com>

#include <cstdio>
#include <string>
#include <vector>

#include <re2/set.h>
#include <re2/stringpiece.h>

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

  static const char* ToString(TypeNo);
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

  static const char* ToString(TypeNo value);
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

  static const char* ToString(Code);
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
  int Init(Error* error = nullptr);

  // Set the contents to be scanned.
  int Begin(const re2::StringPiece& piece);

  // Match and return the next token. Return 0 on succes and -1 on error.
  // if err is not NULL and an error occurs, will be set to a string
  // describing the error message.
  int Pump(Token* tok, Error* error = nullptr);

  // Return the current string piece, mostly for debugging purposes
  re2::StringPiece GetPiece();

 private:
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
int Lex(const re2::StringPiece& source, Token* buf, size_t n,
        Error* error = nullptr);

// Scan/Tokenize the source string until completion; Store the tokens in `buf`.
// Return the number of tokens that were lexed (which may be greater than `n`)
// or -1 on error.
template <size_t N>
int Lex(const re2::StringPiece& source, Token (*buf)[N],
        Error* error = nullptr) {
  return Lex(source, &(*buf)[0], N, error);
}

// Lex the entire source and return 0 if no errors are encountered. Return -1
// and fill `error` if any problems are encountered.
int VerifyLex(const re2::StringPiece& source, Error* error);

// Manages the state machine for parsing JSON structure from a stream of
// tokens
class Parser {
 public:
  enum State {
    PARSING_VALUE = 0,  // Expect '{', '[' or a value literal
    PARSING_KEY,        // Expect a string literal
    PARSING_COLON,      // Expect a ':'
    PARSING_CLOSURE,    // Expect a ']', '}', or ','
    PARSING_ERROR,
  };

  Parser();

  // Reset internal state
  void Reset();

  // Advance the internal parse state with the given token. Return 1 if an
  // actionable event has occured.
  /*
   * \return * 1 - an actionable event has occured. `event` is filled
   *         * 0 - no actionable event
   *         * -1 - an error has occured, `error` is filled if not null
   */
  int HandleToken(const Token& token, Event* event, Error* error = nullptr);

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
  int Init(Error* error = nullptr);
  int Begin(const re2::StringPiece& string);
  int GetNextEvent(Event* event, Error* error = nullptr);

 private:
  Scanner scanner_;
  Parser parser_;
  Token token_;
};

// Scan/Tokenize and Parse the source string until completion; Store the
// parser events in `buf`. Return the number of events that were parsed (which
// may be greater than `n`)  or -1 on error.
int Parse(const re2::StringPiece& source, Event* buf, size_t n,
          Error* error = nullptr);

// Scan/Tokenize and Parse the source string until completion; Store the
// parser events in `buf`. Return the number of events that were parsed (which
// may be greater than `n`)  or -1 on error.
template <size_t N>
int Parse(const re2::StringPiece& source, Event (*buf)[N],
          Error* error = nullptr) {
  return Parse(source, &(*buf)[0], N, error);
}

// Lex and Parse the entire source and return 0 if no errors are encountered.
// Return -1 and fill `error` if any problems are encountered.
int Verify(const re2::StringPiece& source, Error* error = nullptr);

// Options for serialization
struct SerializeOpts {
  size_t indent;          //< Number of spaces to use for indent
  char separators[2][3];  //< map and list separators, i.e. ":" and ","
};

// Manages sequential `printf`s to a single buffer.
class BufPrinter {
 public:
  BufPrinter(char* begin, char* end) : begin_(begin), end_(end), written_(0) {}
  BufPrinter(char* begin, size_t size)
      : begin_(begin), end_(begin + size), written_(0) {}

  template <size_t N>
  explicit BufPrinter(char (*buf)[N])
      : begin_(&(*buf)[0]), end_(&(*buf)[N]), written_(0) {}

  ~BufPrinter() {}

  int operator()(const char* fmt, va_list args);
  int operator()(const char* fmt, ...);
  int operator()(char c);
  int operator()(const re2::StringPiece& str);
  size_t Size();

 private:
  BufPrinter(const BufPrinter&) = delete;
  BufPrinter& operator=(const BufPrinter&) = delete;

  char* begin_;
  char* end_;
  size_t written_;
};

// Stack-guard manages sequential `printf`s to the message buffer and no-ops
// everything if the error pointer is `nullptr`.
class FmtError {
 public:
  FmtError(Error* err, Error::Code code, SourceLocation loc = {});
  FmtError& operator()(const char* fmt, va_list args);
  FmtError& operator()(const char* fmt, ...);
  FmtError& operator()(char c);
  FmtError& operator()(const re2::StringPiece& str);

 private:
  FmtError(const FmtError&) = delete;
  FmtError& operator=(const FmtError&) = delete;

  Error* err_;
  BufPrinter printer_;
};

// Helper function for printing indentations
size_t FmtIndent(BufPrinter* out, size_t indent, size_t depth);

// Safely access elements from a c-style array
template <size_t N>
const char* SafeGet(const char* (&map)[N], size_t idx) {
  if (idx < N) {
    return map[idx];
  } else {
    return "<invalid>";
  }
}

// Default serialization option
extern SerializeOpts kDefaultOpts;

}  // namespace json
