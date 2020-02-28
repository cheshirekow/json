// Copyright (C) 2012 Josh Bialkowski (josh.bialkowski@gmail.com)

/**
 *  @file
 *  @date   Aug 4, 2013
 *  @author Josh Bialkowski (josh.bialkowski@gmail.com)
 */

#include <cstdint>
#include <iterator>

#include "util/redblack.h"

namespace redblack {

// ----------------------------------------------------------------------------
//        Node
// ----------------------------------------------------------------------------

Node::Node()
    : color(Color::BLACK), parent(nullptr), left(nullptr), right(nullptr) {}

// ----------------------------------------------------------------------------
//        Iterator
// ----------------------------------------------------------------------------

Iterator::Iterator(Tree* tree, Node* node) : tree(tree), node(node) {}

Node* Iterator::operator*() {
  return node;
}

Iterator& Iterator::operator++() {
  node = tree->tree_successor(node);
  return *this;
}

Iterator& Iterator::operator--() {
  node = tree->tree_predecessor(node);
  return *this;
}

bool Iterator::operator!=(const Iterator& other) const {
  return node != other.node;
}

// ----------------------------------------------------------------------------
//        ConstIterator
// ----------------------------------------------------------------------------

ConstIterator::ConstIterator(const Tree* tree, const Node* node)
    : tree(tree), node(node) {}

const Node* ConstIterator::operator*() {
  return node;
}

ConstIterator& ConstIterator::operator++() {
  node = tree->tree_successor(node);
  return *this;
}

ConstIterator& ConstIterator::operator--() {
  node = tree->tree_predecessor(node);
  return *this;
}

bool ConstIterator::operator!=(const ConstIterator& other) const {
  return node != other.node;
}

// ----------------------------------------------------------------------------
//        Tree
// ----------------------------------------------------------------------------

Tree::Tree() : root_(&nil_), size_(0) {
  nil_.left = nil_.right = nil_.parent = &nil_;
  nil_.color = Color::BLACK;
}

void Tree::clear() {
  root_ = &nil_;
  size_ = 0;
}

void Tree::left_rotate(Node* x) {
  Node* y = x->right;  //< set y
  x->right = y->left;  //< turn y's left subtree into x's right
                       //  subtree
  y->left->parent = x;
  y->parent = x->parent;

  if (x->parent == &nil_)
    root_ = y;
  else if (x == x->parent->left)
    x->parent->left = y;
  else
    x->parent->right = y;

  y->left = x;  //< put x on y's left
  x->parent = y;
}

void Tree::right_rotate(Node* x) {
  Node* y = x->left;   //< set y
  x->left = y->right;  //< turn y's left subtree into x's left
                       //  subtree
  y->right->parent = x;
  y->parent = x->parent;

  if (x->parent == &nil_)
    root_ = y;
  else if (x == x->parent->right)
    x->parent->right = y;
  else
    x->parent->left = y;

  y->right = x;  //< put x on y's right
  x->parent = y;
}

void Tree::insert_fixup(Node* z) {
  while (z->parent->color == Color::RED) {
    if (z->parent == z->parent->parent->left) {
      Node* y = z->parent->parent->right;
      if (y->color == Color::RED) {
        z->parent->color = Color::BLACK;
        y->color = Color::BLACK;
        z->parent->parent->color = Color::RED;
        z = z->parent->parent;
      } else {
        if (z == z->parent->right) {
          z = z->parent;
          left_rotate(z);
        }
        z->parent->color = Color::BLACK;
        z->parent->parent->color = Color::RED;
        right_rotate(z->parent->parent);
      }
    } else {
      Node* y = z->parent->parent->left;
      if (y->color == Color::RED) {
        z->parent->color = Color::BLACK;
        y->color = Color::BLACK;
        z->parent->parent->color = Color::RED;
        z = z->parent->parent;
      } else {
        if (z == z->parent->left) {
          z = z->parent;
          right_rotate(z);
        }
        z->parent->color = Color::BLACK;
        z->parent->parent->color = Color::RED;
        left_rotate(z->parent->parent);
      }
    }
  }
  root_->color = Color::BLACK;
}

void Tree::insert(Node* z) {
  ++size_;

  Node* y = &nil_;
  Node* x = root_;
  while (x != &nil_) {
    y = x;
    if (less_than(z, x)) {
      x = x->left;
    } else {
      x = x->right;
    }
  }

  z->parent = y;
  if (y == &nil_) {
    root_ = z;
  } else if (less_than(z, y)) {
    y->left = z;
  } else {
    y->right = z;
  }

  z->left = &nil_;
  z->right = &nil_;

  if (y) {
    z->color = Color::RED;
    insert_fixup(z);
  } else {
    z->color = Color::BLACK;
  }
}

void Tree::remove_fixup(Node* x) {
  while (x != root_ && x->color == Color::BLACK) {
    if (x == x->parent->left) {
      Node* w = x->parent->right;
      if (w->color == Color::RED) {
        w->color = Color::BLACK;
        x->parent->color = Color::RED;
        left_rotate(x->parent);
        w = x->parent->right;
      }

      if (w->left->color == Color::BLACK && w->right->color == Color::BLACK) {
        w->color = Color::RED;
        x = x->parent;
      } else {
        if (w->right->color == Color::BLACK) {
          w->left->color = Color::BLACK;
          w->color = Color::RED;
          right_rotate(w);
          w = x->parent->right;
        }
        w->color = x->parent->color;
        x->parent->color = Color::BLACK;
        w->right->color = Color::BLACK;
        left_rotate(x->parent);
        x = root_;
      }
    } else {
      Node* w = x->parent->left;
      if (w->color == Color::RED) {
        w->color = Color::BLACK;
        x->parent->color = Color::RED;
        right_rotate(x->parent);
        w = x->parent->left;
      }

      if (w->right->color == Color::BLACK && w->left->color == Color::BLACK) {
        w->color = Color::RED;
        x = x->parent;
      } else {
        if (w->left->color == Color::BLACK) {
          w->right->color = Color::BLACK;
          w->color = Color::RED;
          left_rotate(w);
          w = x->parent->left;
        }
        w->color = x->parent->color;
        x->parent->color = Color::BLACK;
        w->left->color = Color::BLACK;
        right_rotate(x->parent);
        x = root_;
      }
    }
  }
  x->color = Color::BLACK;
}

Node* Tree::tree_minimum(Node* x) {
  while (x->left != &nil_) {
    x = x->left;
  }
  return x;
}

const Node* Tree::tree_minimum(const Node* x) const {
  while (x->left != &nil_) {
    x = x->left;
  }
  return x;
}

const Node* Tree::tree_minimum() const {
  return tree_minimum(&nil_);
}

Node* Tree::tree_maximum(Node* x) {
  while (x->right != &nil_) {
    x = x->right;
  }
  return x;
}

const Node* Tree::tree_maximum(const Node* x) const {
  while (x->right != &nil_) {
    x = x->right;
  }
  return x;
}

const Node* Tree::tree_maximum() const {
  return tree_maximum(&nil_);
}

Node* Tree::tree_successor(Node* x) {
  if (x->right != &nil_) {
    return tree_minimum(x->right);
  }
  Node* y = x->parent;
  while (y != &nil_ && x == y->right) {
    x = y;
    y = y->parent;
  }
  return y;
}

const Node* Tree::tree_successor(const Node* x) const {
  if (x->right != &nil_) {
    return tree_minimum(x->right);
  }
  const Node* y = x->parent;
  while (y != &nil_ && x == y->right) {
    x = y;
    y = y->parent;
  }
  return y;
}

Node* Tree::tree_predecessor(Node* x) {
  if (x->left != &nil_) {
    return tree_maximum(x->left);
  }
  Node* y = x->parent;
  while (y != &nil_ && x == y->left) {
    x = y;
    y = y->parent;
  }
  return y;
}

const Node* Tree::tree_predecessor(const Node* x) const {
  if (x->left != &nil_) {
    return tree_maximum(x->left);
  }
  const Node* y = x->parent;
  while (y != &nil_ && x == y->left) {
    x = y;
    y = y->parent;
  }
  return y;
}

void Tree::remove(Node* z) {
  --size_;

  Node* y;
  if (z->left == &nil_ || z->right == &nil_) {
    // z is leafish (it is either a leaf node, or a parent of a single ndoe),
    // meaning that rotating it out of the tree is trivial, as we simply move
    // it's one child up the tree
    y = z;
  } else {
    // z is a fully interior node so move down the tree to find the next
    // node in-order. That node is necessarily a leafish node (property of
    // BSTs, otherwise it would have a left child, and that left child would be
    // less than the found node, and greater than z, which is a contradiction).
    // We are going to swap the removed node with this node, which would keep
    // the tree consistent at this interior location.
    y = tree_successor(z);
  }

  // Note that `y` is leafish so at most one of it's children is not null
  Node* x = &nil_;
  if (y->left != &nil_) {
    x = y->left;
  } else if (y->right != &nil_) {
    x = y->right;
  }

  // `y` is going to move. If it's the leafish success of the node to remove,
  // then it is going to jump up the tree. If it's the node to remove, then
  // it just goes away. In either case, if it has a child that child will be
  // orphaned, so we need to reparent it.
  if (x != &nil_) {
    x->parent = y->parent;
  }

  if (y->parent == &nil_) {
    root_ = x;
  } else if (y == y->parent->left) {
    y->parent->left = x;
  } else {
    y->parent->right = x;
  }

  // y's previous slot has been taken by x (which may be null) and x is
  // orphaned out of the tree, so let's clear out it's pointers
  y->left = nullptr;
  y->right = nullptr;
  y->parent = nullptr;

  if (y != z) {
    // If y is the leafish successor of z, then we need to swap it into
    // z's slot. Otherwise y is z itself and we can skip this step
    std::swap(z->left, y->left);
    std::swap(z->right, y->right);
    std::swap(z->parent, y->parent);
    std::swap(z->color, y->color);
  }

  // If we moved a node `x` up the tree, and the node that vacated that slot
  // was a black node, then the black depth of all the nodes in this subtree
  // has changed... so we need to re-balance and re-color it.
  if (x != &nil_ && y->color == Color::BLACK) {
    remove_fixup(x);
  }
}

Iterator Tree::begin() {
  return Iterator(this, tree_minimum(root_));
}

Iterator Tree::rbegin() {
  return Iterator(this, tree_maximum(root_));
}

Iterator Tree::end() {
  return Iterator(this, &nil_);
}

ConstIterator Tree::begin() const {
  return ConstIterator(this, tree_minimum(root_));
}

ConstIterator Tree::rbegin() const {
  return ConstIterator(this, tree_maximum(root_));
}

ConstIterator Tree::end() const {
  return ConstIterator(this, &nil_);
}

size_t Tree::size() const {
  return size_;
}

bool Tree::is_empty() const {
  return size() == 0;
}

Tree::operator bool() const {
  return !is_empty();
}

Node* Tree::pop_front() {
  if (root_ == &nil_) {
    return nullptr;
  }

  Node* node = tree_minimum(root_);
  remove(node);
  return node;
}

Node* Tree::pop_back() {
  if (root_ == &nil_) {
    return nullptr;
  }

  Node* node = tree_maximum(root_);
  remove(node);
  return node;
}

}  // namespace redblack
