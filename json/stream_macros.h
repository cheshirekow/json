#pragma once
// Copyright 2018 Josh Bialkowski <josh.bialkowski@gmail.com>

// ----------------------------------------------------------------------------
// Begin: Ugly Macro Implementation Details
// ----------------------------------------------------------------------------

// NOTE(josh): inspired by https://stackoverflow.com/a/11640759/141023

// Defer concat of two tokens through one layer of macro indirection so that
// arguments are expanded before they are concatenated.
// See https://gcc.gnu.org/onlinedocs/gcc-2.95.3/cpp_1.html#SEC26
#define JSON_PRIMITIVE_CAT(a, ...) a##__VA_ARGS__
#define JSON_CAT(a, ...) JSON_PRIMITIVE_CAT(a, __VA_ARGS__)

// Evaluates to the second argument
#define JSON_TAKE_TWO_(x, n, ...) n

// Evaluates to the second argument. Ensures that there are at least two
// arguments by appending "0" to the argument list. If there was only one
// argument, will evaluate to zero.
#define JSON_TAKE_TWO(...) JSON_TAKE_TWO_(__VA_ARGS__, 0, )

// Expands a single token into two tokens, the second being "1"
#define JSON_PAD_ONE(x) x, 1,

// Magic, see IS_PAREN for what this is doing. This is a macro which we will
// use as a macro only in the case of a placeholder entry in the argument
// list. The technique is described in
// https://gcc.gnu.org/onlinedocs/gcc-2.95.3/cpp_1.html#SEC12
#define JSON_PAREN_PROBE(...) JSON_PAD_ONE(~)

// If x is a pair of parentheses (literally "()") then evaluates to 1. Otherwise
// evaluates to 0.
//
// Expansion if the argument is a paren-pair is:
//
//  IS_PAREN(())
//   => TAKE_TWO(PAREN_PROBE())
//   => TAKE_TWO(PAD_ONE(~))
//   => TAKE_TWO(~,1)
//   => TAKE_TWO_(~,1,0)
//   => 1
//
// If x is any other <token>, then the argument of TAKE_TWO will be
// "IS_PAREN <token>". The expansion then is
//
// IS_PAREN(<token>)
//  => TAKE_TWO(IS_PAREN <token>)
//  => TAKE_TWO_(IS_PAREN <token>, 0)
//  => 0
#define JSON_IS_PAREN(x) JSON_TAKE_TWO(JSON_PAREN_PROBE x)

// Our macro will expand cases for a fixed number of arguments. We will pad the
// variable argument list with this extra token sequence.
#define JSON_FILL() \
  (), (), (), (), (), (), (), (), (), (), (), (), (), (), (), (), (), (), (), ()

// This is the actual meat of the macro system: simply a case statement to
// dispatch the appropriate parser for the given field (by name)
#define JSON_CASE_IF_NOT_PAREN_0(key)     \
  case Hash(#key):                        \
    ParseValue(event, stream, &out->key); \
    break;

// This is the else-case for when the parameter is a placeholder
#define JSON_CASE_IF_NOT_PAREN_1(key)

// Evaluates to a case statement unless x is a placeholder in the form of
// parenthesis: "()".
#define JSON_CASE_IF_NOT_PAREN(x) \
  JSON_CAT(JSON_CASE_IF_NOT_PAREN_, JSON_IS_PAREN(x))(x)

// Generate a case statement for the first N elements in the list
#define JSON_MAKE_CASES_N(_00, _01, _02, _03, _04, _05, _06, _07, _08, _09, \
                          _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, \
                          ...)                                              \
  JSON_CASE_IF_NOT_PAREN(_00)                                               \
  JSON_CASE_IF_NOT_PAREN(_01)                                               \
  JSON_CASE_IF_NOT_PAREN(_02)                                               \
  JSON_CASE_IF_NOT_PAREN(_03)                                               \
  JSON_CASE_IF_NOT_PAREN(_04)                                               \
  JSON_CASE_IF_NOT_PAREN(_05)                                               \
  JSON_CASE_IF_NOT_PAREN(_06)                                               \
  JSON_CASE_IF_NOT_PAREN(_07)                                               \
  JSON_CASE_IF_NOT_PAREN(_08)                                               \
  JSON_CASE_IF_NOT_PAREN(_09)                                               \
  JSON_CASE_IF_NOT_PAREN(_10)                                               \
  JSON_CASE_IF_NOT_PAREN(_11)                                               \
  JSON_CASE_IF_NOT_PAREN(_12)                                               \
  JSON_CASE_IF_NOT_PAREN(_13)                                               \
  JSON_CASE_IF_NOT_PAREN(_14)                                               \
  JSON_CASE_IF_NOT_PAREN(_15)                                               \
  JSON_CASE_IF_NOT_PAREN(_16)                                               \
  JSON_CASE_IF_NOT_PAREN(_17)                                               \
  JSON_CASE_IF_NOT_PAREN(_18)                                               \
  JSON_CASE_IF_NOT_PAREN(_19)

// One layer of macro indirectino, presumably to glue together to halfs of
// the macro argument list
#define JSON_MAKE_CASES_(...) JSON_MAKE_CASES_N(__VA_ARGS__)

// Generate a case statement for each argument
#define JSON_MAKE_CASES(...) JSON_MAKE_CASES_(__VA_ARGS__, JSON_FILL())

#define JSON_EMIT_IF_NOT_PAREN_0(key) \
  EmitField(#key, value.key, opts, depth, out);

#define JSON_EMIT_IF_NOT_PAREN_1(key)
#define JSON_EMIT_IF_NOT_PAREN(x) \
  JSON_CAT(JSON_EMIT_IF_NOT_PAREN_, JSON_IS_PAREN(x))(x)

#define JSON_EMIT_WSEP_IF_NOT_PAREN_0(key) \
  EmitFieldSep(opts, out);                 \
  EmitField(#key, value.key, opts, depth, out);

#define JSON_EMIT_WSEP_IF_NOT_PAREN_1(key)
#define JSON_EMIT_WSEP_IF_NOT_PAREN(x) \
  JSON_CAT(JSON_EMIT_WSEP_IF_NOT_PAREN_, JSON_IS_PAREN(x))(x)

#define JSON_MAKE_EMITS_N(_00, _01, _02, _03, _04, _05, _06, _07, _08, _09, \
                          _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, \
                          ...)                                              \
  JSON_EMIT_IF_NOT_PAREN(_00)                                               \
  JSON_EMIT_WSEP_IF_NOT_PAREN(_01)                                          \
  JSON_EMIT_WSEP_IF_NOT_PAREN(_02)                                          \
  JSON_EMIT_WSEP_IF_NOT_PAREN(_03)                                          \
  JSON_EMIT_WSEP_IF_NOT_PAREN(_04)                                          \
  JSON_EMIT_WSEP_IF_NOT_PAREN(_05)                                          \
  JSON_EMIT_WSEP_IF_NOT_PAREN(_06)                                          \
  JSON_EMIT_WSEP_IF_NOT_PAREN(_07)                                          \
  JSON_EMIT_WSEP_IF_NOT_PAREN(_08)                                          \
  JSON_EMIT_WSEP_IF_NOT_PAREN(_09)                                          \
  JSON_EMIT_WSEP_IF_NOT_PAREN(_10)                                          \
  JSON_EMIT_WSEP_IF_NOT_PAREN(_11)                                          \
  JSON_EMIT_WSEP_IF_NOT_PAREN(_12)                                          \
  JSON_EMIT_WSEP_IF_NOT_PAREN(_13)                                          \
  JSON_EMIT_WSEP_IF_NOT_PAREN(_14)                                          \
  JSON_EMIT_WSEP_IF_NOT_PAREN(_15)                                          \
  JSON_EMIT_WSEP_IF_NOT_PAREN(_16)                                          \
  JSON_EMIT_WSEP_IF_NOT_PAREN(_17)                                          \
  JSON_EMIT_WSEP_IF_NOT_PAREN(_18)                                          \
  JSON_EMIT_WSEP_IF_NOT_PAREN(_19)                                          \
  if (opts.indent) {                                                        \
    (*out)("\n");                                                           \
  }

#define JSON_MAKE_EMITS_(...) JSON_MAKE_EMITS_N(__VA_ARGS__)
#define JSON_MAKE_EMITS(...) JSON_MAKE_EMITS_(__VA_ARGS__, JSON_FILL())

// ----------------------------------------------------------------------------
// End: Ugly Macro Implementation Details
// ----------------------------------------------------------------------------

#define JSON_DECLPARSE(OutType)                                           \
  namespace json {                                                        \
  namespace stream {                                                      \
  int ParseField(const re2::StringPiece& key, const Event& event,         \
                 LexerParser* stream, OutType* out);                      \
  void ParseValue(const Event& event, LexerParser* stream, OutType* out); \
  } /* namespace stream */                                                \
  } /* namespace json */

#define JSON_DEFNPARSE(OutType, ...)                                       \
  namespace json {                                                         \
  namespace stream {                                                       \
  int ParseField(const re2::StringPiece& key, const Event& event,          \
                 LexerParser* stream, OutType* out) {                      \
    uint64_t keyid = RuntimeHash(key);                                     \
    switch (keyid) {                                                       \
      JSON_MAKE_CASES(__VA_ARGS__)                                         \
      default:                                                             \
        SinkValue(event, stream);                                          \
        return 1;                                                          \
    }                                                                      \
    return 0;                                                              \
  }                                                                        \
                                                                           \
  void ParseValue(const Event& event, LexerParser* stream, OutType* out) { \
    ParseObject(event, stream, out);                                       \
  }                                                                        \
  } /* namespace stream */                                                 \
  } /* namespace json */

#define JSON_DECL(OutType) \
  JSON_DECLPARSE(OutType)  \
  JSON_DECLEMIT(OutType)

#define JSON_DECLEMIT(OutType)                                    \
  namespace json {                                                \
  namespace stream {                                              \
  void EmitValue(const OutType& value, const SerializeOpts& opts, \
                 size_t depth, BufPrinter* out);                  \
  } /* namespace stream */                                        \
  } /* namespace json */

#define JSON_DEFNEMIT(OutType, ...)                               \
  namespace json {                                                \
  namespace stream {                                              \
  void EmitValue(const OutType& value, const SerializeOpts& opts, \
                 size_t depth, BufPrinter* out) {                 \
    (*out)("{");                                                  \
    if (opts.indent) {                                            \
      (*out)("\n");                                               \
    }                                                             \
    JSON_MAKE_EMITS(__VA_ARGS__);                                 \
    FmtIndent(out, opts.indent, depth);                           \
    (*out)("}");                                                  \
  }                                                               \
  } /* namespace stream */                                        \
  } /* namespace json */

#define JSON_DEFN(OutType, ...)        \
  JSON_DEFNPARSE(OutType, __VA_ARGS__) \
  JSON_DEFNEMIT(OutType, __VA_ARGS__)
