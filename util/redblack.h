// Copyright (C) 2012 Josh Bialkowski (josh.bialkowski@gmail.com)

/**
 *  @file
 *  @date   Aug 4, 2013
 *  @author Josh Bialkowski (josh.bialkowski@gmail.com)
 */

#pragma once

#include <cstdint>
#include <iterator>

namespace redblack {

enum struct Color : uint8_t { BLACK, RED };

/// A node in a redblack tree
struct Node {
  Color color;
  Node* parent;
  Node* left;
  Node* right;
  Node();
};

class Tree;

/// an iterator which resolves nodes of a red-black tree in-order
struct Iterator {
  Tree* tree;
  Node* node;

  Iterator(Tree* tree, Node* node);
  Node* operator*();
  Iterator& operator++();
  Iterator& operator--();
  bool operator!=(const Iterator& other) const;
};

/// an iterator which resolves nodes of a red-black tree in-order
struct ConstIterator {
  const Tree* tree;
  const Node* node;

  ConstIterator(const Tree* tree, const Node* node);
  const Node* operator*();
  ConstIterator& operator++();
  ConstIterator& operator--();
  bool operator!=(const ConstIterator& other) const;
};

/// implements red black trees from CLRS
class Tree {
 public:
  Tree();
  void clear();
  void insert(Node* z);
  void remove(Node* z);

  Iterator begin();
  Iterator rbegin();
  Iterator end();

  ConstIterator begin() const;
  ConstIterator rbegin() const;
  ConstIterator end() const;

  size_t size() const;

  /// Return the node in the tree with minimum value
  /// @see CLRS 12.2 page 258
  Node* tree_minimum(Node* x);
  const Node* tree_minimum(const Node* x) const;
  const Node* tree_minimum() const;

  /// Return the node in the tree with maximum value
  /// @see CLRS 12.2 page 258
  Node* tree_maximum(Node* x);
  const Node* tree_maximum(const Node* x) const;
  const Node* tree_maximum() const;

  /// Return the node in the tree that comes immediately after `x` in sorted
  /// order. i.e. The node with minimum value greater than the value at `x`
  /// @see CLRS 12.2 page 259
  Node* tree_successor(Node* x);
  const Node* tree_successor(const Node* x) const;

  /// Return the nod ein the tree that comes immediately before `x` in sorted
  /// order. i.e. the node in the tree with maximum value less than the value
  /// at `x`
  /// @see CLRS 12.2 page 259
  Node* tree_predecessor(Node* x);
  const Node* tree_predecessor(const Node* x) const;

  /// Lookup the value of each node using whatever implementation-specific
  /// mechanism, and return true if the value of `x` is less than the value of
  /// `y`. Defined in derived classes.
  virtual bool less_than(const Node* x, const Node* y) const = 0;

  /// Return true if the tree contains no nodes
  bool is_empty() const;

  /// Return true if the tree contains no nodes
  operator bool() const;

  /// Remove and return the smallest-valued node in the tree
  Node* pop_front();

  /// Remove and return the largest-valued node in the tree
  Node* pop_back();

 private:
  Node nil_;
  Node* root_;
  size_t size_;

  /// Move x one level down the tree to the left, swapping it with its left
  /// child.
  void left_rotate(Node* x);

  /// Move x one level down the tree to the right, swapping it with its right
  /// child.
  void right_rotate(Node* x);

  void insert_fixup(Node* z);
  void remove_fixup(Node* x);
};

}  // namespace redblack
