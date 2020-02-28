// Copyright 2018 Josh Bialkowski <josh.bialkowski@gmail.com>
#include <fstream>

#include <gtest/gtest.h>

#include "argue/argue.h"

#if __clang__
#  define USE_DESIGNATED_INITIALIZERS 1
#elif __GNUC__
#  if __GNUC__ > 5 && __cplusplus > 201400L
#    define USE_DESIGNATED_INITIALIZERS 1
#  else
#    define USE_DESIGNATED_INITIALIZERS 0
#  endif
#else
#  define USE_DESIGNATED_INITIALIZERS 0
#endif

void ResetParser(argue::Parser* parser, const argue::Parser::Metadata& meta) {
  using argue::Parser;
  (parser)->~Parser();
  new (parser) Parser(meta);
}

void ResetParser(argue::Parser* parser) {
  using argue::Parser;
  (parser)->~Parser();
  new (parser) Parser();
}

TEST(StoreTest, StoreScalar) {
  std::ofstream nullstream{"/dev/null"};
  int foo = 0;
  argue::Parser parser;

  // Expect failure if we have too few args, manditory positional argument
  // remains
  ResetParser(&parser);
  parser.AddArgument("foo", &foo, {});
  foo = 0;
  EXPECT_EQ(argue::PARSE_EXCEPTION, parser.ParseArgs({}, &nullstream));
  EXPECT_EQ(0, foo);

  // Expect failure if we have too many args, which ensures that the parser
  // does not consume the remaining args.
  ResetParser(&parser);
  parser.AddArgument("foo", &foo, {});
  foo = 0;
  EXPECT_EQ(argue::PARSE_EXCEPTION, parser.ParseArgs({"1", "2"}, &nullstream));
  EXPECT_EQ(1, foo);

  // Expect failure if we have too many args, which ensures that the parser
  // does not consume the remaining args.
  ResetParser(&parser);
  parser.AddArgument("foo", &foo, {});
  foo = 0;
  EXPECT_EQ(argue::PARSE_FINISHED, parser.ParseArgs({"1"}, &nullstream));
  EXPECT_EQ(1, foo);

  // Flags default optional so an empty args list should be OK
  ResetParser(&parser);
  parser.AddArgument("-f", "--foo", &foo, {});
  foo = 0;
  EXPECT_EQ(argue::PARSE_FINISHED, parser.ParseArgs({}, &nullstream));
  EXPECT_EQ(0, foo);

  // Expect failure if we have too many args, which ensures that the parser
  // does not consume the remaining args.
  ResetParser(&parser);
  parser.AddArgument("-f", "--foo", &foo, {});
  foo = 0;
  EXPECT_EQ(argue::PARSE_EXCEPTION, parser.ParseArgs({"1", "2"}, &nullstream));
  EXPECT_EQ(0, foo);

  ResetParser(&parser);
  parser.AddArgument("-f", "--foo", &foo, {});
  foo = 0;
  EXPECT_EQ(argue::PARSE_FINISHED, parser.ParseArgs({"-f", "1"}, &nullstream));
  EXPECT_EQ(1, foo);

  ResetParser(&parser);
  parser.AddArgument("-f", "--foo", &foo, {});
  foo = 0;
  EXPECT_EQ(argue::PARSE_FINISHED,
            parser.ParseArgs({"--foo", "1"}, &nullstream));
  EXPECT_EQ(1, foo);

  // Ensure that flag deduction works correctly
  ResetParser(&parser);
  parser.AddArgument("-f", &foo, {});
  foo = 0;
  EXPECT_EQ(argue::PARSE_FINISHED, parser.ParseArgs({"-f", "1"}, &nullstream));
  EXPECT_EQ(1, foo);

  ResetParser(&parser);
  parser.AddArgument("--foo", &foo, {});
  foo = 0;
  EXPECT_EQ(argue::PARSE_FINISHED,
            parser.ParseArgs({"--foo", "1"}, &nullstream));
  EXPECT_EQ(1, foo);

  // If argument is optional then parse should not fail on empty string
  ResetParser(&parser);
  parser.AddArgument("foo", &foo,
                     {.action = "store", .nargs = argue::ZERO_OR_ONE});
  foo = 0;
  EXPECT_EQ(argue::PARSE_FINISHED, parser.ParseArgs({}, &nullstream));
  EXPECT_EQ(0, foo);
}

TEST(StoreTest, StoreTypes) {
  std::ofstream nullstream{"/dev/null"};
  argue::Parser parser;

  ResetParser(&parser);
  int32_t i32_foo = 0;
  parser.AddArgument("foo", &i32_foo, {});
  EXPECT_EQ(argue::PARSE_FINISHED, parser.ParseArgs({"123"}, &nullstream));
  EXPECT_EQ(123, i32_foo);

  ResetParser(&parser);
  uint32_t u32_foo = 0;
  parser.AddArgument("foo", &u32_foo, {});
  EXPECT_EQ(argue::PARSE_FINISHED, parser.ParseArgs({"123"}, &nullstream));
  EXPECT_EQ(123, u32_foo);

  ResetParser(&parser);
  float f32_foo = 0;
  parser.AddArgument("foo", &f32_foo, {});
  EXPECT_EQ(argue::PARSE_FINISHED, parser.ParseArgs({"123"}, &nullstream));
  EXPECT_EQ(123, f32_foo);

  ResetParser(&parser);
  double f64_foo = 0;
  parser.AddArgument("foo", &f64_foo, {});
  EXPECT_EQ(argue::PARSE_FINISHED, parser.ParseArgs({"123"}, &nullstream));
  EXPECT_EQ(123, f64_foo);

  ResetParser(&parser);
  std::string str_foo = "hello";
  parser.AddArgument("foo", &str_foo, {});
  EXPECT_EQ(argue::PARSE_FINISHED, parser.ParseArgs({"123"}, &nullstream));
  EXPECT_EQ("123", str_foo);
}

TEST(StoreTest, StoreOneOrMore) {
  std::ofstream nullstream{"/dev/null"};
  std::list<int> container;
  std::list<int> expected;
  const std::list<int> empty_list;
  argue::Parser parser;

  // An empty argument list should fail if the requirement is one or more
  ResetParser(&parser);
#if USE_DESIGNATED_INITIALIZERS
  parser.AddArgument<int>("foo",
                          {.action = "store",  //
                           .nargs = argue::ONE_OR_MORE,
                           .dest = &container});
#else
  parser.AddArgument<int>("foo", {/*.action =*/"store",            //
                                  /*.nargs =*/argue::ONE_OR_MORE,  //
                                  ARGUE_EMPTY3,
                                  /*.dest =*/&container});
#endif
  container = {};
  EXPECT_EQ(argue::PARSE_EXCEPTION, parser.ParseArgs({}, &nullstream));
  EXPECT_EQ(empty_list, container);

  ResetParser(&parser);
#if USE_DESIGNATED_INITIALIZERS
  parser.AddArgument<int>("foo",
                          {.action = "store",  //
                           .nargs = argue::ONE_OR_MORE,
                           .dest = &container});
#else
  parser.AddArgument<int>("foo", {/*.action =*/"store",            //
                                  /*.nargs =*/argue::ONE_OR_MORE,  //
                                  ARGUE_EMPTY3,
                                  /*.dest =*/&container});
#endif
  container = {};
  EXPECT_EQ(argue::PARSE_FINISHED, parser.ParseArgs({"1"}, &nullstream));
  expected = {1};
  EXPECT_EQ(expected, container);

  ResetParser(&parser);
#if USE_DESIGNATED_INITIALIZERS
  parser.AddArgument<int>("foo",
                          {.action = "store",  //
                           .nargs = argue::ONE_OR_MORE,
                           .dest = &container});
#else
  parser.AddArgument<int>("foo", {/*.action =*/"store",            //
                                  /*.nargs =*/argue::ONE_OR_MORE,  //
                                  ARGUE_EMPTY3,
                                  /*.dest =*/&container});
#endif
  container = {};
  EXPECT_EQ(argue::PARSE_FINISHED,
            parser.ParseArgs({"1", "2", "3"}, &nullstream));
  expected = {1, 2, 3};
  EXPECT_EQ(expected, container);

  ResetParser(&parser);
  parser.AddArgument("foo", &container,
                     {.action = "store",  //
                      .nargs = argue::ONE_OR_MORE});
  container = {};
  EXPECT_EQ(argue::PARSE_FINISHED,
            parser.ParseArgs({"1", "2", "3"}, &nullstream));
  expected = {1, 2, 3};
  EXPECT_EQ(expected, container);
}

TEST(StoreTest, StoreZeroOrMore) {
  std::ofstream nullstream{"/dev/null"};
  std::list<int> container;
  std::list<int> expected;
  const std::list<int> empty_list;
  argue::Parser parser;

  // An empty argument list is allowed if specification is for zero or more
  ResetParser(&parser);
  parser.AddArgument("foo", &container,
                     {.action = "store", .nargs = argue::ZERO_OR_MORE});
  container = {};
  EXPECT_EQ(argue::PARSE_FINISHED, parser.ParseArgs({}, &nullstream));
  EXPECT_EQ(empty_list, container);

  ResetParser(&parser);
  parser.AddArgument("foo", &container,
                     {.action = "store", .nargs = argue::ZERO_OR_MORE});
  container = {};
  EXPECT_EQ(argue::PARSE_FINISHED, parser.ParseArgs({"1"}, &nullstream));
  expected = {1};
  EXPECT_EQ(expected, container);

  ResetParser(&parser);
  parser.AddArgument("foo", &container,
                     {.action = "store", .nargs = argue::ZERO_OR_MORE});
  container = {};
  EXPECT_EQ(argue::PARSE_FINISHED,
            parser.ParseArgs({"1", "2", "3"}, &nullstream));
  expected = {1, 2, 3};
  EXPECT_EQ(expected, container);
}

TEST(StoreTest, StoreFixedSize) {
  std::ofstream nullstream{"/dev/null"};
  std::stringstream strm;
  int dummy;
  std::list<int> container;
  std::list<int> expected;
  const std::list<int> empty_list;
  argue::Parser parser;

  // An empty argument list is allowed if specification is for zero or more
  ResetParser(&parser);
  EXPECT_THROW(
      parser.AddArgument("foo", &container, {.action = "store", .nargs = 0}),
      argue::Exception);
  // container = {};
  // EXPECT_EQ(argue::PARSE_EXCEPTION, parser.ParseArgs({}));
  // EXPECT_EQ(empty_list, container);

  ResetParser(&parser);
  parser.AddArgument("foo", &container, {.action = "store", .nargs = 1});
  container = {};
  EXPECT_EQ(argue::PARSE_FINISHED, parser.ParseArgs({"1"}, &strm))
      << strm.str();
  strm.str("");
  expected = {1};
  EXPECT_EQ(expected, container);

  ResetParser(&parser);
  parser.AddArgument("foo", &container, {.action = "store", .nargs = 1});
  parser.AddArgument("bar", &dummy, {});
  container = {};
  dummy = 0;
  EXPECT_EQ(argue::PARSE_FINISHED, parser.ParseArgs({"1", "2"}, &nullstream));
  expected = {1};
  EXPECT_EQ(expected, container);
  EXPECT_EQ(2, dummy);

  ResetParser(&parser);
  parser.AddArgument("foo", &container, {.action = "store", .nargs = 3});
  parser.AddArgument("bar", &dummy, {});
  container = {};
  dummy = 0;
  EXPECT_EQ(argue::PARSE_FINISHED,
            parser.ParseArgs({"1", "2", "3", "4"}, &nullstream));
  expected = {1, 2, 3};
  EXPECT_EQ(expected, container);
  EXPECT_EQ(4, dummy);

  ResetParser(&parser);
  parser.AddArgument("foo", &container, {.action = "store", .nargs = 4});
  container = {};
  EXPECT_EQ(argue::PARSE_EXCEPTION, parser.ParseArgs({"1", "2"}, &nullstream));
  expected = {1, 2};
  EXPECT_EQ(expected, container);
}

TEST(HelpTest, HelpIsDefault) {
  std::ofstream nullstream{"/dev/null"};
  std::stringstream strm;
  argue::Parser parser;
  ResetParser(&parser);
  EXPECT_EQ(argue::PARSE_ABORTED, parser.ParseArgs({"--help"}, &strm))
      << strm.str();
  strm.str("");
  ResetParser(&parser);
  EXPECT_EQ(argue::PARSE_ABORTED, parser.ParseArgs({"-h"}, &strm))
      << strm.str();
  strm.str("");

  ResetParser(&parser, {.add_help = false});
  EXPECT_EQ(argue::PARSE_EXCEPTION, parser.ParseArgs({"--help"}, &nullstream));
  ResetParser(&parser, {.add_help = false});
  EXPECT_EQ(argue::PARSE_EXCEPTION, parser.ParseArgs({"-h"}, &nullstream));
}

TEST(VersionTest, VersionIsDefault) {
  std::ofstream nullstream{"/dev/null"};
  argue::Parser parser;
  ResetParser(&parser);
  EXPECT_EQ(argue::PARSE_ABORTED, parser.ParseArgs({"--version"}, &nullstream));
  ResetParser(&parser);
  EXPECT_EQ(argue::PARSE_ABORTED, parser.ParseArgs({"-v"}, &nullstream));

  ResetParser(&parser, {.add_help = true, .add_version = false});
  EXPECT_EQ(argue::PARSE_EXCEPTION,
            parser.ParseArgs({"--version"}, &nullstream));
  ResetParser(&parser, {.add_help = true, .add_version = false});
  EXPECT_EQ(argue::PARSE_EXCEPTION, parser.ParseArgs({"-v"}, &nullstream));
}

struct TestOpts {
  std::string command;
  std::string foo;

  struct {
    std::string a;
    std::string b;
  } bar;

  struct {
    std::string c;
    std::string d;
  } baz;
};

TEST(SubparserTest, SubparsersWork) {
  TestOpts opts;

  std::stringstream logout;
  argue::Parser parser;
  ResetParser(&parser);
  parser.AddArgument("-f", "--foo", &opts.foo, {});
  auto subparsers = parser.AddSubparsers("command", &opts.command);
  ASSERT_NE(nullptr, subparsers.get());
  auto bar_parser = subparsers->AddParser("bar");
  ASSERT_NE(nullptr, bar_parser.get());
  bar_parser->AddArgument("-a", &opts.bar.a);
  bar_parser->AddArgument("-b", &opts.bar.b);
  auto baz_parser = subparsers->AddParser("baz");
  ASSERT_NE(nullptr, baz_parser.get());
  baz_parser->AddArgument("-c", &opts.baz.c);
  baz_parser->AddArgument("-d", &opts.baz.d);

  ASSERT_EQ(argue::PARSE_FINISHED, parser.ParseArgs({"bar"}, &logout))
      << logout.str();
  EXPECT_EQ("bar", opts.command);

  logout.str("");
  ASSERT_EQ(argue::PARSE_FINISHED,
            parser.ParseArgs({"bar", "-a", "hello", "-b", "world"}, &logout))
      << logout.str();
  EXPECT_EQ("bar", opts.command);
  EXPECT_EQ("hello", opts.bar.a);
  EXPECT_EQ("world", opts.bar.b);

  logout.str("");
  ASSERT_EQ(argue::PARSE_EXCEPTION,
            parser.ParseArgs({"bar", "-c", "hello"}, &logout))
      << logout.str();

  logout.str("");
  ASSERT_EQ(argue::PARSE_FINISHED, parser.ParseArgs({"baz"}, &logout))
      << logout.str();

  logout.str("");
  ASSERT_EQ(argue::PARSE_FINISHED,
            parser.ParseArgs({"baz", "-c", "hello", "-d", "world"}, &logout))
      << logout.str();
  EXPECT_EQ("baz", opts.command);
  EXPECT_EQ("hello", opts.baz.c);
  EXPECT_EQ("world", opts.baz.d);

  logout.str("");
  ASSERT_EQ(argue::PARSE_EXCEPTION,
            parser.ParseArgs({"baz", "-a", "-b"}, &logout))
      << logout.str();

  logout.str("");
  ASSERT_EQ(
      argue::PARSE_FINISHED,
      parser.ParseArgs({"--foo", "hello", "bar", "-a", "hello", "-b", "world"},
                       &logout))
      << logout.str();
}
