// Copyright 2018 Josh Bialkowski <josh.bialkowski@gmail.com>
#include <gtest/gtest.h>

#include "json/builder.h"

using namespace json;  // NOLINT

TEST(BuilderTest, SerializeTest) {
  using namespace json::insource;  // NOLINT
  Variant tree =                   //
      json::Build(O{"hello", 123, "world",
                    O{"foo", O{
                                 "far", 123,      //
                                 "fuz", "hello",  //
                                 "fur", 42.7e2,   //
                                 "fox", true,     //
                                 "fut", false,    //
                                 "fit", nullptr   //
                             }}});

  ASSERT_EQ(tree.typeno, variant::OBJECT);
  ASSERT_EQ(tree["hello"].typeno, variant::INTEGER);
  ASSERT_EQ(tree["world"].typeno, variant::OBJECT);
  ASSERT_EQ(tree["world"]["foo"].typeno, variant::OBJECT);
  ASSERT_EQ(tree["world"]["foo"]["far"].typeno, variant::INTEGER);
  ASSERT_EQ(tree["world"]["foo"]["far"].store.integer, 123);
}

TEST(BuilderTest, InSourceKnownTrees) {
  using namespace json::insource;  // NOLINT
  Variant tree =                   //
      json::Build(O{"hello", 123, "world",
                    O{"foo", O{
                                 "far", 123,      //
                                 "fuz", "hello",  //
                                 "fur", 42.7e2,   //
                                 "fox", true,     //
                                 "fut", false,    //
                                 "fit", nullptr   //
                             }}});

  std::string expect1 =
      "{\"hello\": 123,\"world\": {\"foo\": {\"far\": 123,\"fit\": "
      "null,\"fox\": true,\"fur\": 4270.000000,\"fut\": false,\"fuz\": "
      "\"hello\"}}}";
  std::string expect2 =
      ""
      "{\n"
      "  \"hello\": 123,\n"
      "  \"world\": {\n"
      "    \"foo\": {\n"
      "      \"far\": 123,\n"
      "      \"fit\": null,\n"
      "      \"fox\": true,\n"
      "      \"fur\": 4270.000000,\n"
      "      \"fut\": false,\n"
      "      \"fuz\": \"hello\"\n"
      "    }\n"
      "  }\n"
      "}";

  std::vector<char> buf;
  buf.resize(256, 0);

  size_t size1 = tree.Serialize(
      &buf[0], &buf.back(),
      json::SerializeOpts{.indent = 0, .separators = {": ", ","}});
  EXPECT_EQ(size1, 116);
  EXPECT_EQ(expect1, std::string(&buf[0]));

  size_t size2 = tree.Serialize(
      &buf[0], &buf.back(),
      json::SerializeOpts{.indent = 2, .separators = {": ", ","}});
  EXPECT_EQ(size2, 178);
  EXPECT_EQ(expect2, std::string(&buf[0]));
}
