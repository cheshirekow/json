#pragma once
// Copyright 2018 Josh Bialkowski <josh.bialkowski@gmail.com>
#include "json/type_registry.h"

// ----------------------------------------------------------------------------
// Begin: Ugly Macro Implementation Details
// ----------------------------------------------------------------------------

// NOTE(josh): inspired by https://stackoverflow.com/a/11640759/141023

// Defer concat of two tokens through one layer of macro indirection so that
// arguments are expanded before they are concatenated.
// See https://gcc.gnu.org/onlinedocs/gcc-2.95.3/cpp_1.html#SEC26
#define JSON_PRIMITIVE_CAT(a, ...) a##__VA_ARGS__
#define JSON_CAT(a, ...) JSON_PRIMITIVE_CAT(a, __VA_ARGS__)

// Evaluates to the first argument
#define JSON_TAKE_ONE(x, ...) x

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

// -----------------------------------------------------------------------------
// JSON_MAKE_CASES()
// -----------------------------------------------------------------------------

// This is the actual meat of the macro system: simply a case statement to
// dispatch the appropriate parser for the given field (by name)
#define JSON_CASE_IF_NOT_PAREN_0(key)        \
  case json::hash(#key):                     \
    registry.parse_value(stream, &out->key); \
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

// One layer of macro indirection, to glue together two halfs of
// the macro argument list
#define JSON_MAKE_CASES_(...) JSON_MAKE_CASES_N(__VA_ARGS__)

// Generate a case statement for each argument
#define JSON_MAKE_CASES(...) JSON_MAKE_CASES_(__VA_ARGS__, JSON_FILL())

// -----------------------------------------------------------------------------
// JSON_MAKE_DUMPS()
// -----------------------------------------------------------------------------
// See JSON_MAKE_CASES() above for a description of the structure of this
// macro system. The following are not commented.

#define JSON_DUMP_IF_NOT_PAREN_0(key) \
  result |= dumper->dump_field(#key, value.key);

#define JSON_DUMP_IF_NOT_PAREN_1(key)

#define JSON_DUMP_IF_NOT_PAREN(x) \
  JSON_CAT(JSON_DUMP_IF_NOT_PAREN_, JSON_IS_PAREN(x))(x)

#define JSON_MAKE_DUMPS_N(_00, _01, _02, _03, _04, _05, _06, _07, _08, _09, \
                          _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, \
                          ...)                                              \
  JSON_DUMP_IF_NOT_PAREN(_00)                                               \
  JSON_DUMP_IF_NOT_PAREN(_01)                                               \
  JSON_DUMP_IF_NOT_PAREN(_02)                                               \
  JSON_DUMP_IF_NOT_PAREN(_03)                                               \
  JSON_DUMP_IF_NOT_PAREN(_04)                                               \
  JSON_DUMP_IF_NOT_PAREN(_05)                                               \
  JSON_DUMP_IF_NOT_PAREN(_06)                                               \
  JSON_DUMP_IF_NOT_PAREN(_07)                                               \
  JSON_DUMP_IF_NOT_PAREN(_08)                                               \
  JSON_DUMP_IF_NOT_PAREN(_09)                                               \
  JSON_DUMP_IF_NOT_PAREN(_10)                                               \
  JSON_DUMP_IF_NOT_PAREN(_11)                                               \
  JSON_DUMP_IF_NOT_PAREN(_12)                                               \
  JSON_DUMP_IF_NOT_PAREN(_13)                                               \
  JSON_DUMP_IF_NOT_PAREN(_14)                                               \
  JSON_DUMP_IF_NOT_PAREN(_15)                                               \
  JSON_DUMP_IF_NOT_PAREN(_16)                                               \
  JSON_DUMP_IF_NOT_PAREN(_17)                                               \
  JSON_DUMP_IF_NOT_PAREN(_18)                                               \
  JSON_DUMP_IF_NOT_PAREN(_19)

#define JSON_MAKE_DUMPS_(...) JSON_MAKE_DUMPS_N(__VA_ARGS__)
#define JSON_MAKE_DUMPS(...) JSON_MAKE_DUMPS_(__VA_ARGS__, JSON_FILL())

// -----------------------------------------------------------------------------
// JSON_MAKE_REGISTRATIONS()
// -----------------------------------------------------------------------------
// See JSON_MAKE_CASES() above for a description of the structure of this
// macro system. The following are not commented.

#define JSON_REGISTER_IF_NOT_PAREN_0(SUFFIX)                \
  registry->register_object(JSON_CAT(parsefields_, SUFFIX), \
                            JSON_CAT(dumpfields_, SUFFIX));

#define JSON_REGISTER_IF_NOT_PAREN_1(key)

#define JSON_REGISTER_IF_NOT_PAREN(x) \
  JSON_CAT(JSON_REGISTER_IF_NOT_PAREN_, JSON_IS_PAREN(x))(x)

#define JSON_MAKE_REGISTRATIONS_N(_00, _01, _02, _03, _04, _05, _06, _07, _08, \
                                  _09, _10, _11, _12, _13, _14, _15, _16, _17, \
                                  _18, _19, ...)                               \
  JSON_REGISTER_IF_NOT_PAREN(_00)                                              \
  JSON_REGISTER_IF_NOT_PAREN(_01)                                              \
  JSON_REGISTER_IF_NOT_PAREN(_02)                                              \
  JSON_REGISTER_IF_NOT_PAREN(_03)                                              \
  JSON_REGISTER_IF_NOT_PAREN(_04)                                              \
  JSON_REGISTER_IF_NOT_PAREN(_05)                                              \
  JSON_REGISTER_IF_NOT_PAREN(_06)                                              \
  JSON_REGISTER_IF_NOT_PAREN(_07)                                              \
  JSON_REGISTER_IF_NOT_PAREN(_08)                                              \
  JSON_REGISTER_IF_NOT_PAREN(_09)                                              \
  JSON_REGISTER_IF_NOT_PAREN(_10)                                              \
  JSON_REGISTER_IF_NOT_PAREN(_11)                                              \
  JSON_REGISTER_IF_NOT_PAREN(_12)                                              \
  JSON_REGISTER_IF_NOT_PAREN(_13)                                              \
  JSON_REGISTER_IF_NOT_PAREN(_14)                                              \
  JSON_REGISTER_IF_NOT_PAREN(_15)                                              \
  JSON_REGISTER_IF_NOT_PAREN(_16)                                              \
  JSON_REGISTER_IF_NOT_PAREN(_17)                                              \
  JSON_REGISTER_IF_NOT_PAREN(_18)                                              \
  JSON_REGISTER_IF_NOT_PAREN(_19)

#define JSON_MAKE_REGISTRATIONS_(...) JSON_MAKE_REGISTRATIONS_N(__VA_ARGS__)
#define JSON_MAKE_REGISTRATIONS(...) \
  JSON_MAKE_REGISTRATIONS_(__VA_ARGS__, JSON_FILL())

// ----------------------------------------------------------------------------
// End: Ugly Macro Implementation Details
// ----------------------------------------------------------------------------

#define JSON_DECLPARSE_(ValueType, SUFFIX)                                 \
  int JSON_CAT(parsefields_, SUFFIX)(                                      \
      const json::stream::Registry& registry, const re2::StringPiece& key, \
      json::LexerParser* stream, ValueType* out);

// JSON_DECLPARSE(<outtype>, [suffix])
//  where:
//  * <outtype> is the type description of the output structure, usually the
//    name of a class or struct, but could also be a decltype(...)
//  * <suffix> is an optional suffix to use in the naming of the generated
//    functions. If not supplied, it defaults to <outtype>, but you must be
//    provide the optional argument if <outtype> contains any special
//    characters (like "::" or "()").
//
// Generates a function declaration
//   int parsefields_SUFFIX(
//    const Registry& registry, const re2::StringPiece& key,
//    LexerParser* stream, ValueType* out);
//
// which implements the required client API for registration with the
// serializable-type registry.
#define JSON_DECLPARSE(...)                   \
  JSON_DECLPARSE_(JSON_TAKE_ONE(__VA_ARGS__), \
                  JSON_TAKE_TWO(__VA_ARGS__, __VA_ARGS__))

// JSON_DEFNPARSE(<outtype>, <suffix>, field0, [field1, [field2, ...]])
//  where:
//  * <outtype> is the type description of the output structure, usually the
//    name of a class or struct, but could also be a decltype(...)
//  * <namesuffix> is a suffix to use in the naming of the generated
//    functions.
//
// Generates a function definition
//   int parsefields_SUFFIX(
//    const Registry& registry, const re2::StringPiece& key,
//    LexerParser* stream, ValueType* out);
// which implements the required client API for registration with the
// serializable-type registry.
//
// Note that SUFFIX must match between JSON_DECLPARSE and JSON_DEFNPARSE
// for the associated pair.
#define JSON_DEFNPARSE(ValueType, SUFFIX, ...)                             \
  int JSON_CAT(parsefields_, SUFFIX)(                                      \
      const json::stream::Registry& registry, const re2::StringPiece& key, \
      json::LexerParser* stream, ValueType* out) {                         \
    uint64_t keyid = json::runtime_hash(key);                              \
    switch (keyid) {                                                       \
      JSON_MAKE_CASES(__VA_ARGS__)                                         \
      default:                                                             \
        json::sink_value(stream);                                          \
        return 1;                                                          \
    }                                                                      \
    return 0;                                                              \
  }

#define JSON_DECLDUMP_(ValueType, SUFFIX)                   \
  int JSON_CAT(dumpfields_, SUFFIX)(const ValueType& value, \
                                    json::stream::Dumper* dumper);

// JSON_DECLDUMP(<outtype>, [suffix])
//  where:
//  * <outtype> is the type description of the output structure, usually the
//    name of a class or struct, but could also be a decltype(...)
//  * <suffix> is an optional suffix to use in the naming of the generated
//    functions. If not supplied, it defaults to <outtype>, but you must be
//    provide the optional argument if <outtype> contains any special
//    characters (like "::" or "()").
//
// Generates a function declaration
//   int dumpfields_SUFFIX(
//    const ValueType& value, Dumper* dumper);
//
// which implements the required client API for registration with the
// serializable-type registry.
#define JSON_DECLDUMP(...)                   \
  JSON_DECLDUMP_(JSON_TAKE_ONE(__VA_ARGS__), \
                 JSON_TAKE_TWO(__VA_ARGS__, __VA_ARGS__))

// JSON_DEFNDUMP(<outtype>, <suffix>, field0, [field1, [field2, ...]])
//  where:
//  * <outtype> is the type description of the output structure, usually the
//    name of a class or struct, but could also be a decltype(...)
//  * <suffix> is an optional suffix to use in the naming of the generated
//    functions. If not supplied, it defaults to <outtype>, but you must be
//    provide the optional argument if <outtype> contains any special
//    characters (like "::" or "()").
//
// Generates a function definition
//   int dumpfields_SUFFIX(
//    const ValueType& value, Dumper* dumper);
//
// which implements the required client API for registration with the
// serializable-type registry.
//
// Note that SUFFIX must match between JSON_DECLDUMP and JSON_DEFNDUMP
// for the associated pair.
#define JSON_DEFNDUMP(ValueType, SUFFIX, ...)                       \
  int JSON_CAT(dumpfields_, SUFFIX)(const ValueType& value,         \
                                    json::stream::Dumper* dumper) { \
    int result = 0;                                                 \
    JSON_MAKE_DUMPS(__VA_ARGS__);                                   \
    return result;                                                  \
  }

#define JSON_DECL_(OutType, SUFFIX) \
  JSON_DECLPARSE(OutType, SUFFIX)   \
  JSON_DECLDUMP(OutType, SUFFIX)

// JSON_DECL(<outtype>, [suffix])
//  where:
//  * <outtype> is the type description of the output structure, usually the
//    name of a class or struct, but could also be a decltype(...)
//  * <suffix> is an optional suffix to use in the naming of the generated
//    functions. If not supplied, it defaults to <outtype>, but you must be
//    provide the optional argument if <outtype> contains any special
//    characters (like "::" or "()").
//
// Generates a pair of function declarations
//   int parsefields_SUFFIX(
//    const Registry& registry, const re2::StringPiece& key,
//    LexerParser* stream, ValueType* out);
//   int dumpfields_SUFFIX(
//    const ValueType& value, Dumper* dumper);
//
// which implement the required client API for registration with the
// serializable-type registry.
#define JSON_DECL(...)                   \
  JSON_DECL_(JSON_TAKE_ONE(__VA_ARGS__), \
             JSON_TAKE_TWO(__VA_ARGS__, __VA_ARGS__))

// JSON_DEFN(<outtype>)
//  where:
//  * <outtype> is the type description of the output structure, usually the
//    name of a class or struct, but could also be a decltype(...)
//
// Generates a pair of function definitions
//   int parsefields_<outtype>(
//    const Registry& registry, const re2::StringPiece& key,
//    LexerParser* stream, ValueType* out);
//   int dumpfields_<outtype>(
//    const ValueType& value, Dumper* dumper);
//
// which implement the required client API for registration with the
// serializable-type registry.
//
// Note that this macro uses the name of the output type as the suffix for
// the generated function names. If the output type is not composed of a single
// preprocessor token then you'll need to use the macro that takes two required
// parameters JSON_DEFN2.
#define JSON_DEFN(OutType, ...)                 \
  JSON_DEFNPARSE(OutType, OutType, __VA_ARGS__) \
  JSON_DEFNDUMP(OutType, OutType, __VA_ARGS__)

// JSON_DEFN(<outtype>, <suffix>)
//  where:
//  * <outtype> is the type description of the output structure, usually the
//    name of a class or struct, but could also be a decltype(...)
//  * <suffix> is the suffix to use in the naming of the generated
//    functions.
//
// Generates a pair of function definitions
//   int parsefields_<outtype>(
//    const Registry& registry, const re2::StringPiece& key,
//    LexerParser* stream, ValueType* out);
//   int dumpfields_<outtype>(
//    const ValueType& value, Dumper* dumper);
//
// which implement the required client API for registration with the
// serializable-type registry.
//
// Note that this macro requires you to specify the suffix for the generated
// function names. If <outtype> is a single preprocessor token you can use that
// as the suffix for the generated function names in which case you can use
// JSON_DEFN isntead and avoid specifying the argument twice.
#define JSON_DEFN2(OutType, SUFFIX, ...)       \
  JSON_DEFNPARSE(OutType, SUFFIX, __VA_ARGS__) \
  JSON_DEFNDUMP(OutType, SUFFIX, __VA_ARGS__)

// JSON_DEFN_REGISTRATION_FN(<suffix>, <suffix0>, [<suffix1>, [<suffix2>, ...]])
//
// Generate a function definition for:
//  int register_types_SUFFIX(Registry* registry);
//
// which registers several JSON-serializable objects with `registry`. The
// additional parameters <suffix0>...<suffixn> are the suffixes used when
// defining the parse/dump functions. i.e. they are the suffixes used in
// JSON_DEFN2.
#define JSON_DEFN_REGISTRATION_FN(SUFFIX, ...)                               \
  int JSON_CAT(register_types_, SUFFIX)(json::stream::Registry * registry) { \
    JSON_MAKE_REGISTRATIONS(__VA_ARGS__);                                    \
    return 0;                                                                \
  }

// JSON_REGISTER_GLOBALLY(<suffix>, <suffix0>, [<suffix1>, [<suffix2>, ...]])
//
// Generate a function definition for:
//  int register_types_SUFFIX(Registry* registry);
//
// which registers several JSON-serializable objects with `registry`. The
// additional parameters <suffix0>...<suffixn> are the suffixes used when
// defining the parse/dump functions. i.e. they are the suffixes used in
// JSON_DEFN2.
//
// And also define a dummy global which is assigned the output of that
// function. As a result, each of the JSON-serializable types will be
// registered globally during program static initialization.
#define JSON_REGISTER_GLOBALLY(SUFFIX, ...)      \
  JSON_DEFN_REGISTRATION_FN(SUFFIX, __VA_ARGS__) \
  static int JSON_CAT(kDummy_, SUFFIX) =         \
      JSON_CAT(register_types_, SUFFIX)(json::stream::global_registry());
