// Copyright 2018 Josh Bialkowski <josh.bialkowski@gmail.com>
#include <gtest/gtest.h>

#include "tangent/json/variant.h"

using namespace json;  // NOLINT

TEST(VariantTest, ManuallyBuildKnownTrees) {
  Variant bar{static_cast<int64_t>(123)};
  Variant foo{variant::OBJECT};
  foo["bar"] = bar;
  Variant hello{variant::OBJECT};
  hello["foo"] = foo;

  ASSERT_EQ(foo.typeno, variant::OBJECT);
  ASSERT_EQ(foo["bar"].typeno, variant::INTEGER);
  ASSERT_EQ(foo["bar"].store.integer, 123);
  ASSERT_EQ(hello.typeno, variant::OBJECT);
  ASSERT_EQ(hello["foo"].typeno, variant::OBJECT);
  ASSERT_EQ(hello["foo"]["bar"].typeno, variant::INTEGER);
  ASSERT_EQ(hello["foo"]["bar"].store.integer, 123);
}
