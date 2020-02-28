// Copyright 2018 Josh Bialkowski <josh.bialkowski@gmail.com>
#include <gtest/gtest.h>

#include "json/json.h"
#include "json/type_registry.h"

struct TestA {
  int field_a;
  float field_b;
  double field_c;
  bool field_d;
};

namespace json {
int parsefield_TestA(const stream::Registry& registry,
                     const re2::StringPiece& key, LexerParser* stream,
                     TestA* out) {
  // TODO(josh): we should return the result of parse_value right?
  uint64_t keyid = runtime_hash(key);
  switch (keyid) {
    case hash("field_a"):
      registry.parse_value(stream, &out->field_a);
      break;
    case hash("field_b"):
      registry.parse_value(stream, &out->field_b);
      break;
    case hash("field_c"):
      registry.parse_value(stream, &out->field_c);
      break;
    case hash("field_d"):
      registry.parse_value(stream, &out->field_d);
      break;
    default:
      sink_value(stream);
      return 1;
  }
  return 0;
}

int dumpfields_TestA(const TestA& value, stream::Dumper* dumper) {
  int result = 0;
  result |= dumper->dump_field("field_a", value.field_a);
  result |= dumper->dump_field("field_b", value.field_b);
  result |= dumper->dump_field("field_c", value.field_c);
  result |= dumper->dump_field("field_d", value.field_d);
  return result;
}

static const int _dummy0 = stream::global_registry()->register_object(
    parsefield_TestA, dumpfields_TestA);
}  // namespace json

TEST(TypeRegistryTest, TestParseSimpleStruct) {
  std::string test_string =
      "{"
      " \"field_a\": 1,"
      " \"field_b\": 2.0,"
      " \"field_c\": 3.0,"
      " \"field_d\": true "
      "}";

  TestA value{};
  CHECK_EQ(0, json::stream::parse(test_string, &value));
  CHECK_EQ(1, value.field_a);
  CHECK_EQ(2.0, value.field_b);
  CHECK_EQ(3.0, value.field_c);
  CHECK_EQ(true, value.field_d);
}

struct TestB {
  TestA field_a;
  int field_b;
};

TEST(TypeRegistryTest, TestDumpSimpleStruct) {
  std::stringstream outstream;
  json::stream::StreamDumper dumper{&outstream};
  TestA value{1, 2.0, 3.0, true};
  EXPECT_EQ(0, json::stream::dump(&dumper, value));

  std::string expect =
      "{\n"
      "  \"field_a\": 1,\n"
      "  \"field_b\": 2,\n"
      "  \"field_c\": 3,\n"
      "  \"field_d\": true\n"
      "}";
  EXPECT_EQ(expect, outstream.str());
}

namespace json {
int parsefield_TestB(const stream::Registry& registry,
                     const re2::StringPiece& key, LexerParser* stream,
                     TestB* out) {
  uint64_t keyid = runtime_hash(key);
  switch (keyid) {
    case hash("field_a"):
      registry.parse_value(stream, &out->field_a);
      break;
    case hash("field_b"):
      registry.parse_value(stream, &out->field_b);
      break;
    default:
      sink_value(stream);
      return 1;
  }
  return 0;
}
static const int _dummy1 =
    stream::global_registry()->register_object(parsefield_TestB);
}  // namespace json

TEST(TypeRegistryTest, TestNestedStruct) {
  std::string test_string =
      "{"
      "  \"field_a\": {"
      "    \"field_a\": 1,"
      "    \"field_b\": 2.0,"
      "    \"field_c\": 3.0,"
      "    \"field_d\": true "
      "  }, "
      "  \"field_b\": 4"
      "}";

  TestB value{};
  CHECK_EQ(0, json::stream::parse(test_string, &value));
  CHECK_EQ(1, value.field_a.field_a);
  CHECK_EQ(2.0, value.field_a.field_b);
  CHECK_EQ(3.0, value.field_a.field_c);
  CHECK_EQ(true, value.field_a.field_d);
  CHECK_EQ(4, value.field_b);
}

TEST(TypeRegistryTest, TestSimpleList) {
  std::string test_string = "[1, 2, 3, 4, 5]";
  int values[] = {0, 0, 0, 0, 0, 0};

  CHECK_EQ(0, json::stream::parse(test_string, &values));
  CHECK_EQ(1, values[0]);
  CHECK_EQ(2, values[1]);
  CHECK_EQ(3, values[2]);
  CHECK_EQ(4, values[3]);
  CHECK_EQ(5, values[4]);
  CHECK_EQ(0, values[5]);
}
