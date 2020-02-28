// Copyright 2018 Josh Bialkowski <josh.bialkowski@gmail.com>
#include <array>

#include <gtest/gtest.h>
#include <re2/re2.h>

#include "json/item.h"

using namespace json;  // NOLINT

TEST(ParserTest, Re2CanParseJSONNumbers) {
  int64_t integer;
  EXPECT_TRUE(RE2::FullMatch("-123456", "([-\\d]+)", &integer));
  EXPECT_EQ(-123456, integer);

  double value;
  EXPECT_TRUE(RE2::FullMatch("123.456", "(.+)", &value));
  EXPECT_EQ(123.456, value);

  EXPECT_TRUE(RE2::FullMatch("-123.456", "(.+)", &value));
  EXPECT_EQ(-123.456, value);

  EXPECT_TRUE(RE2::FullMatch("1.2e3", "(.+)", &value));
  EXPECT_EQ(1.2e3, value);

  EXPECT_TRUE(RE2::FullMatch("-1.2e-3", "(.+)", &value));
  EXPECT_EQ(-1.2e-3, value);

  EXPECT_TRUE(RE2::FullMatch("1.2e+3", "(.+)", &value));
  EXPECT_EQ(1.2e3, value);
}

std::array<item::Item, 255> g_item_store_;
std::array<Token, 255> g_token_store_;
std::array<Event, 255> g_event_store_;

TEST(ParserTest, TestKnownParsings) {
  Error error{};
  std::string test_string =
      "{\"foo\":{\"bar\":1,\"baz\":[\"a\",1,12.3,true,false,null]}}";

  int nevents =
      Parse(test_string, &g_event_store_[0], g_event_store_.size(), &error);
  ASSERT_EQ(16, nevents);
  ASSERT_EQ(Event::OBJECT_BEGIN, g_event_store_[0].typeno);
  ASSERT_EQ(Event::OBJECT_KEY, g_event_store_[1].typeno);
  ASSERT_EQ(Event::OBJECT_BEGIN, g_event_store_[2].typeno);
  ASSERT_EQ(Event::OBJECT_KEY, g_event_store_[3].typeno);
  ASSERT_EQ(Event::VALUE_LITERAL, g_event_store_[4].typeno);
  ASSERT_EQ(Event::OBJECT_KEY, g_event_store_[5].typeno);
  ASSERT_EQ(Event::LIST_BEGIN, g_event_store_[6].typeno);
  ASSERT_EQ(Event::VALUE_LITERAL, g_event_store_[7].typeno);
  ASSERT_EQ(Event::VALUE_LITERAL, g_event_store_[8].typeno);
  ASSERT_EQ(Event::VALUE_LITERAL, g_event_store_[9].typeno);
  ASSERT_EQ(Event::VALUE_LITERAL, g_event_store_[10].typeno);
  ASSERT_EQ(Event::VALUE_LITERAL, g_event_store_[11].typeno);
  ASSERT_EQ(Event::VALUE_LITERAL, g_event_store_[12].typeno);
  ASSERT_EQ(Event::LIST_END, g_event_store_[13].typeno);
  ASSERT_EQ(Event::OBJECT_END, g_event_store_[14].typeno);
  ASSERT_EQ(Event::OBJECT_END, g_event_store_[15].typeno);
}

TEST(ItemParserTest, TestKnownParsings) {
  Error err{};
  std::string test_string =
      "{\"foo\":{\"bar\":1,\"baz\":[\"a\",1,12.3,true,false,null]}}";

  int ntokens =
      Lex(test_string, &g_token_store_[0], g_token_store_.size(), &err);
  ASSERT_LT(0, ntokens);
  ASSERT_LT(ntokens, g_token_store_.size());

  Error parse_error{};
  item::ItemParser parser(g_item_store_.begin(), g_item_store_.end());
  for (size_t idx = 0; idx < static_cast<size_t>(ntokens); ++idx) {
    ASSERT_EQ(0, parser.Consume(g_token_store_[idx], &parse_error))
        << "Error(" << parse_error.code << "): " << parse_error.msg
        << " for token " << idx;
  }

  item::Item* object = &g_item_store_[0];
  ASSERT_EQ(item::Item::JSON_OBJECT, object->typeno);

  item::Item* child = object->AsGroup()->head_;
  ASSERT_NE(nullptr, child);
  ASSERT_EQ(item::Item::JSON_KEY, child->typeno);
  ASSERT_EQ(std::string("foo"), child->store.string);
  ASSERT_EQ("foo", child->store.string);

  child = child->next;
  ASSERT_NE(nullptr, child);
  ASSERT_EQ(item::Item::JSON_OBJECT, child->typeno);

  object = child;
  child = object->AsGroup()->head_;
  ASSERT_NE(nullptr, child);
  ASSERT_EQ(item::Item::JSON_KEY, child->typeno);

  child = child->next;
  ASSERT_NE(nullptr, child);
  ASSERT_EQ(item::Item::JSON_INTEGER, child->typeno);

  child = child->next;
  ASSERT_NE(nullptr, child);
  ASSERT_EQ(item::Item::JSON_KEY, child->typeno);

  child = child->next;
  ASSERT_NE(nullptr, child);
  ASSERT_EQ(item::Item::JSON_LIST, child->typeno);
  ASSERT_EQ(nullptr, child->next);

  child = child->AsGroup()->head_;
  ASSERT_NE(nullptr, child);
  ASSERT_EQ(item::Item::JSON_STRING, child->typeno);

  child = child->next;
  ASSERT_NE(nullptr, child);
  ASSERT_EQ(item::Item::JSON_INTEGER, child->typeno);

  child = child->next;
  ASSERT_NE(nullptr, child);
  ASSERT_EQ(item::Item::JSON_FLOAT, child->typeno);

  child = child->next;
  ASSERT_NE(nullptr, child);
  ASSERT_EQ(item::Item::JSON_BOOLEAN, child->typeno);

  child = child->next;
  ASSERT_NE(nullptr, child);
  ASSERT_EQ(item::Item::JSON_BOOLEAN, child->typeno);

  child = child->next;
  ASSERT_NE(nullptr, child);
  ASSERT_EQ(item::Item::JSON_NULL, child->typeno);

  item::Item& root = g_item_store_[0];
  EXPECT_EQ(item::Item::JSON_OBJECT, root["foo"].typeno);
  EXPECT_EQ(item::Item::JSON_INTEGER, root["foo"]["bar"].typeno);
  EXPECT_EQ(1, root["foo"]["bar"].store.integer);
  EXPECT_EQ(12.3, root["foo"]["baz"][2].store.floatval);
  EXPECT_EQ(item::Item::JSON_INVALID, root["random"]["keys"].typeno);
}
