// Copyright 2018 Josh Bialkowski <josh.bialkowski@gmail.com>
#include <gtest/gtest.h>

#include <array>
#include <cstdint>
#include <cstring>

#include "util/container_of.h"
#include "util/redblack.h"

struct TestNode {
  int value;
  redblack::Node link;
};

class TestTree : public redblack::Tree {
 public:
  bool less_than(const redblack::Node* x,
                 const redblack::Node* y) const override {
    const TestNode* tx = container_of(x, &TestNode::link);
    const TestNode* ty = container_of(y, &TestNode::link);
    return (tx->value < ty->value);
  }

 private:
};

enum { NUM_NODES = 10 };

TEST(RedBlackTest, SimpleSortTest) {
  std::vector<int> values = {8,  2,  17, 3, 15, 7, 0,  10, 13, 4,
                             18, 11, 16, 9, 12, 6, 14, 19, 5,  1};

  TestTree tree;
  for (int value : values) {
    TestNode* node = new TestNode();
    node->value = value;
    tree.insert(&node->link);
  }

  auto iter = tree.begin();
  for (size_t idx = 0; idx < 20; idx++) {
    ASSERT_NE(iter, tree.end());
    TestNode* node = container_of(*iter, &TestNode::link);
    EXPECT_EQ(node->value, idx);
    ++iter;
  }

  for (size_t idx = 0; idx < 20; idx++) {
    ASSERT_TRUE(tree);
    TestNode* node = container_of(tree.pop_front(), &TestNode::link);
    EXPECT_EQ(node->value, idx);
    delete node;
  }
}
