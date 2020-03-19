// Copyright 2018 Josh Bialkowski <josh.bialkowski@gmail.com>
#include <array>
#include <cstdint>
#include <list>
#include <map>
#include <vector>

#include <gtest/gtest.h>
#include <re2/stringpiece.h>

#include "tangent/json/stream_macros.h"
#include "tangent/json/type_registry.h"

struct TestA {
  struct {
    int a = 1;
    double b = 3.14;
    float e = 1.2;
    int f = 3;
  } foo;

  struct {
    int c = 2;
    float d = 3.2f;
  } bar;

  struct Boz {
    int a = 1;
    float b = 2.0;
  } boz[2];
};

struct TestB {
  int8_t a = 1;
  int16_t b = 2;
  int32_t c = 3;
  int64_t d = 4;
  uint8_t e = 5;
  uint16_t f = 6;
  uint32_t g = 8;
  float h = 9.0f;
  double i = 10.0;
  bool j = true;
  int8_t k[4] = {1, 2, 3, 4};
  char l[10] = "hello";

  struct {
    int8_t a = 1;
    int8_t b = 2;
  } xbar;

  struct XBaz {
    int8_t a = 1;
    int8_t b = 2;
  } xbaz[3];
};

struct TestC {
  int32_t a = 1;
  int32_t b = 2;

  struct C {
    float d = 3.0f;
    float e = 4.0f;

    struct F {
      uint32_t g = 5;
    } f;
  } c;
};

static JSON_DEFN2(decltype(TestA::foo), FOO, a, b, e, f);
static JSON_DEFN2(decltype(TestA::bar), BAR, c, d);
static JSON_DEFN2(TestA::Boz, BOZ, a, b);
static JSON_DEFN(TestA, foo, bar, boz);

static JSON_DEFN2(decltype(TestB::xbar), XBAR, a, b);
static JSON_DEFN2(TestB::XBaz, XBAZ, a, b);
static JSON_DEFN(TestB, a, b, c, d, e, f, g, h, i, j, k, l, xbar, xbaz);

static JSON_DEFN2(TestC::C::F, TESTC_C_F, g);
static JSON_DEFN2(TestC::C, TESTC_C, d, e, f);
static JSON_DEFN(TestC, a, b, c);

static JSON_REGISTER_GLOBALLY(ANYTHING, TestA, FOO, BAR, BOZ, TestB, XBAR, XBAZ,
                              TestC, TESTC_C_F, TESTC_C);

TEST(StreamTest, BasicTest) {
  const char test_str[] =
      "{\"foo\": {\"a\": 2, \"e\": 42.0}, \"bar\": {\"d\": 6.1},"
      " \"boz\": [{\"a\": 2, \"b\": 3.0}, {\"b\": 1.0}]}";
  TestA obj;

  json::stream::parse(test_str, &obj);
  EXPECT_EQ(2, obj.foo.a);
  EXPECT_EQ(3.14, obj.foo.b);
  EXPECT_EQ(42.0, obj.foo.e);
  EXPECT_EQ(3, obj.foo.f);
  EXPECT_EQ(2, obj.bar.c);
  EXPECT_EQ(6.1f, obj.bar.d);
  EXPECT_EQ(2, obj.boz[0].a);
  EXPECT_EQ(3.0f, obj.boz[0].b);
  EXPECT_EQ(1, obj.boz[1].a);
  EXPECT_EQ(1.0f, obj.boz[1].b);

  std::string buf = json::stream::dump(obj, json::kCompactOpts);

  const char expect_str[] =
      "{\"foo\":{\"a\":2,\"b\":3.14,\"e\":42,\"f\":3},\"bar\":{"
      "\"c\":2,\"d\":6.1},\"boz\":[{\"a\":2,\"b\":3},{\"a\":1,"
      "\"b\":1}]}";
  EXPECT_EQ(expect_str, std::string(&buf[0]));

  std::string buf2 = json::stream::dump(obj, json::kDefaultOpts);
  const char expect_str2[] =
      "{\n"
      "  \"foo\": {\n"
      "    \"a\": 2,\n"
      "    \"b\": 3.14,\n"
      "    \"e\": 42,\n"
      "    \"f\": 3\n"
      "  },\n"
      "  \"bar\": {\n"
      "    \"c\": 2,\n"
      "    \"d\": 6.1\n"
      "  },\n"
      "  \"boz\": [\n"
      "    {\n"
      "      \"a\": 2,\n"
      "      \"b\": 3\n"
      "    },\n"
      "    {\n"
      "      \"a\": 1,\n"
      "      \"b\": 1\n"
      "    }\n"
      "  ]\n"
      "}";
  EXPECT_EQ(expect_str2, buf2);
}

TEST(StreamTest, SerializeTest) {
  TestB obj;
  std::string buf = json::stream::dump(obj, json::kCompactOpts);

  const char expect_str[] =
      "{\"a\":1,\"b\":2,\"c\":3,\"d\":4,\"e\":5,\"f\":6,\"g\":8,\"h\":9,"
      "\"i\":10,\"j\":true,\"k\":[1,2,3,4],\"l\":\"hello\",\"xbar\":{"
      "\"a\":1,\"b\":2},\"xbaz\":[{\"a\":1,\"b\":2},{\"a\":1,\"b\":2},{\"a\":1,"
      "\"b\":2}]}";
  EXPECT_EQ(expect_str, buf);

  std::string buf2 = json::stream::dump(obj, json::kDefaultOpts);
  const char expect_str2[] =
      "{\n"
      "  \"a\": 1,\n"
      "  \"b\": 2,\n"
      "  \"c\": 3,\n"
      "  \"d\": 4,\n"
      "  \"e\": 5,\n"
      "  \"f\": 6,\n"
      "  \"g\": 8,\n"
      "  \"h\": 9,\n"
      "  \"i\": 10,\n"
      "  \"j\": true,\n"
      "  \"k\": [\n"
      "    1,\n"
      "    2,\n"
      "    3,\n"
      "    4\n"
      "  ],\n"
      "  \"l\": \"hello\",\n"
      "  \"xbar\": {\n"
      "    \"a\": 1,\n"
      "    \"b\": 2\n"
      "  },\n"
      "  \"xbaz\": [\n"
      "    {\n"
      "      \"a\": 1,\n"
      "      \"b\": 2\n"
      "    },\n"
      "    {\n"
      "      \"a\": 1,\n"
      "      \"b\": 2\n"
      "    },\n"
      "    {\n"
      "      \"a\": 1,\n"
      "      \"b\": 2\n"
      "    }\n"
      "  ]\n"
      "}";
  EXPECT_EQ(expect_str2, buf2);
}

TEST(StreamTest, ParseTest) {
  const char source_str[] =
      "{\n"
      "  \"a\": 2,\n"
      "  \"b\": 3,\n"
      "  \"c\": 4,\n"
      "  \"d\": 5,\n"
      "  \"e\": 6,\n"
      "  \"f\": 7,\n"
      "  \"g\": 9,\n"
      "  \"h\": 10,\n"
      "  \"i\": 11,\n"
      "  \"j\": false,\n"
      "  \"k\": [\n"
      "    2,\n"
      "    3,\n"
      "    4,\n"
      "    5\n"
      "],\n"
      "  \"l\": \"world\",\n"
      "  \"xbar\": {\n"
      "    \"a\": 2,\n"
      "    \"b\": 3\n"
      "  },\n"
      "  \"xbaz\": [\n"
      "    {\n"
      "      \"a\": 4,\n"
      "      \"b\": 5\n"
      "    },\n"
      "    {\n"
      "      \"a\": 6,\n"
      "      \"b\": 7\n"
      "    },\n"
      "    {\n"
      "      \"a\": 8,\n"
      "      \"b\": 9\n"
      "    }\n"
      "]\n"
      "}";

  TestB obj;
  json::stream::parse(source_str, &obj);
  EXPECT_EQ(2, obj.a);
  EXPECT_EQ(3, obj.b);
  EXPECT_EQ(4, obj.c);
  EXPECT_EQ(5, obj.d);
  EXPECT_EQ(6, obj.e);
  EXPECT_EQ(7, obj.f);
  EXPECT_EQ(9, obj.g);
  EXPECT_EQ(10.0f, obj.h);
  EXPECT_EQ(11.0, obj.i);
  EXPECT_EQ(false, obj.j);
  EXPECT_EQ(2, obj.k[0]);
  EXPECT_EQ(std::string("world"), std::string(obj.l));
}
