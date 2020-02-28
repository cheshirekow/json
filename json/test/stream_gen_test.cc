// Copyright 2018 Josh Bialkowski <josh.bialkowski@gmail.com>
#include <array>
#include <cstdint>
#include <list>
#include <map>
#include <vector>

#include <gtest/gtest.h>
#include <re2/stringpiece.h>

#include "json/test/stream_gen_test.h"
#include "json/test/test_types.h"
#include "json/type_registry.h"

TEST(StreamTest, BasicTest) {
  const char test_str[] =
      "{\"foo\": {\"a\": 2, \"e\": 42.0}, \"bar\": {\"d\": 6.1},"
      " \"boz\": [{\"a\": 2, \"b\": 3.0}, {\"b\": 1.0}]}";
  TestA obj;

  ASSERT_EQ(0, json::stream::parse(test_str, &obj));
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
      "{\"foo\":{\"a\":2,\"b\":3.14,\"e\":42,\"f\":3},"
      "\"bar\":{"
      "\"c\":2,\"d\":6.1},"
      "\"boz\":[{\"a\":2,\"b\":3},{\"a\":1,\"b\":1}]}";
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
  return;
  const char expect_str2[] =
      "{\n"
      "  \"a\": 1,\n"
      "  \"b\": 2,\n"
      "  \"c\": 3,\n"
      "  \"d\": 4,\n"
      "  \"e\": 5,\n"
      "  \"f\": 6,\n"
      "  \"g\": 8,\n"
      "  \"h\": 9.000000,\n"
      "  \"i\": 10.000000,\n"
      "  \"j\": true,\n"
      "  \"k\": [\n"
      "    1,\n"
      "    2,\n"
      "    3,\n"
      "    4\n"
      "],\n"
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
      "]\n"
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
      "  \"h\": 10.000000,\n"
      "  \"i\": 11.000000,\n"
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

class TestDumper : public json::stream::Dumper {
 public:
  TestDumper() : json::stream::Dumper(nullptr, json::kDefaultOpts) {}

  void dump_event(json::stream::DumpEvent::TypeNo eventno) override {
    switch (eventno) {
      case json::stream::DumpEvent::LIST_END:
      case json::stream::DumpEvent::OBJECT_END:
        if (key_path_.size()) {
          key_path_.pop_back();
        }

      default:
        break;
    }
    prev_event_ = eventno;
  }

  void dump_primitive(uint8_t value) override {}
  void dump_primitive(uint16_t value) override {}
  void dump_primitive(uint32_t value) override {
    value_map_[GetActiveKey()] = value;
    key_path_.pop_back();
  }
  void dump_primitive(uint64_t value) override {}
  void dump_primitive(int8_t value) override {}
  void dump_primitive(int16_t value) override {}
  void dump_primitive(int32_t value) override {
    value_map_[GetActiveKey()] = value;
    key_path_.pop_back();
  }
  void dump_primitive(int64_t value) override {}
  void dump_primtiive(float value) override {
    value_map_[GetActiveKey()] = value;
    key_path_.pop_back();
  }
  void dump_primitive(double value) override {
    value_map_[GetActiveKey()] = value;
    key_path_.pop_back();
  }
  void dump_primitive(bool value) override {}
  void dump_primitive(std::nullptr_t nullval) override {}
  void dump_primitive(re2::StringPiece strval) override {
    if (prev_event_ == json::stream::DumpEvent::OBJECT_KEY) {
      key_path_.push_back(strval.as_string());
    }
  }
  void dump_primitive(const std::string& strval) override {
    if (prev_event_ == json::stream::DumpEvent::OBJECT_KEY) {
      key_path_.push_back(strval);
    }
  }
  void dump_primitive(const char* strval) override {
    if (prev_event_ == json::stream::DumpEvent::OBJECT_KEY) {
      key_path_.push_back(strval);
    }
  }

  std::map<std::string, int64_t> value_map_;

 private:
  std::string GetActiveKey() {
    std::stringstream strm;
    auto iter = key_path_.begin();
    if (iter != key_path_.end()) {
      strm << *iter;
      ++iter;
    }
    for (; iter != key_path_.end(); ++iter) {
      strm << "." << *iter;
    }
    return strm.str();
  }

  std::vector<std::string> key_path_;
  json::stream::DumpEvent::TypeNo prev_event_;
};

TEST(StreamTest, CustomDumperTest) {
  TestC test_obj{};
  TestDumper walker;
  ASSERT_EQ(0, json::stream::dump(&walker, test_obj));
  ASSERT_EQ(5, walker.value_map_.size());
  ASSERT_FALSE(walker.value_map_.find("a") == walker.value_map_.end());
  ASSERT_EQ(1, walker.value_map_["a"]);
  ASSERT_FALSE(walker.value_map_.find("b") == walker.value_map_.end());
  ASSERT_EQ(2, walker.value_map_["b"]);
  ASSERT_FALSE(walker.value_map_.find("c.d") == walker.value_map_.end());
  ASSERT_EQ(3, walker.value_map_["c.d"]);
  ASSERT_FALSE(walker.value_map_.find("c.e") == walker.value_map_.end());
  ASSERT_EQ(4, walker.value_map_["c.e"]);
  ASSERT_FALSE(walker.value_map_.find("c.f.g") == walker.value_map_.end());
  ASSERT_EQ(5, walker.value_map_["c.f.g"]);
}
