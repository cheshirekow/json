#pragma once
// Copyright 2018 Josh Bialkowski <josh.bialkowski@gmail.com
#include <sys/types.h>
#include <cassert>

#include "util/container_of.h"

// Simple linked-list based data structures with type-erasure and support
// for multi-list membership.
namespace linked {

// Simple linked list node. Intended to be a member variable of some object
// which is accounted in a linked list.
struct Node {
  Node* prev;
  Node* next;

  Node();
  ~Node();
  void Remove();
  void InsertBefore(Node* other);
  void InsertAfter(Node* other);

  // Turn this node into a single-element ring. Note that this does not
  // Remove() the node if it is currently part of a list and will leave
  // dangling references to this node if called while it is part of an
  // existing list.
  Node* MakeRing();
};

// Simple iterator over nodes
// TODO(josh): add const iterator
struct Iterator {
  bool operator==(const Iterator& other) const;
  bool operator!=(const Iterator& other) const;

  Iterator& operator++();
  Iterator operator++() const;
  Iterator operator+(size_t off);
  Iterator& operator--();
  Iterator operator--() const;
  Iterator operator-(size_t off);
  Node* operator->();
  Node* operator*();

  Node* node;
};

// Simple iterator over nodes
struct ConstIterator {
  bool operator==(const ConstIterator& other) const;
  bool operator!=(const ConstIterator& other) const;
  ConstIterator& operator++();
  ConstIterator operator++() const;
  ConstIterator operator+(size_t off);
  ConstIterator& operator--();
  ConstIterator operator--() const;
  ConstIterator operator-(size_t off);
  const Node* operator->();
  const Node* operator*();

  const Node* node;
};

// Iterate over objects of which the linked::Node is a member
template <class ContainerOf>
struct ContainerIterator {
  ContainerIterator(Node* node, Node ContainerOf::*member);
  bool operator==(const ContainerIterator<ContainerOf>& other) const;
  bool operator!=(const ContainerIterator<ContainerOf>& other) const;
  ContainerIterator<ContainerOf>& operator++();

  ContainerIterator<ContainerOf> operator++() const;
  ContainerIterator<ContainerOf> operator+(size_t off);
  ContainerIterator<ContainerOf>& operator--();
  ContainerIterator<ContainerOf> operator--() const;
  ContainerIterator<ContainerOf> operator-(size_t off);
  ContainerOf* operator->();
  ContainerOf* operator*();

  Node ContainerOf::*member_offset_;
  Node* node_;
};

template <class ContainerOf>
class ContainerRange {
 public:
  ContainerRange(Node* begin, Node* end, Node ContainerOf::*member_offset);
  ContainerIterator<ContainerOf> begin();
  ContainerIterator<ContainerOf> end();

  Node ContainerOf::*member_offset_;
  Node* begin_;
  Node* end_;
};

// Simple linked-list. Is actually a doubly-linked ring but one node in
// the ring is special, and stored within this object. That special node
// is the sentinel for start/end of the list
struct List {
 public:
  List();
  Iterator begin();
  Iterator rbegin();
  Iterator end();
  ConstIterator begin() const;
  ConstIterator rbegin() const;
  ConstIterator end() const;
  void PushBack(Node* node);
  void PushFront(Node* node);
  Node* PopFront();
  Node* PopBack();
  Node* Front();
  Node* Back();
  bool IsEmpty() const;
  operator bool() const;
  void Clear();

  // Move all nodes from other into this one
  void StealNodesFrom(List* other);
  void GiveNodesTo(List* other);

  template <class ContainerOf>
  ContainerRange<ContainerOf> IterContainers(Node ContainerOf::*member_offset);

 protected:
  Node sentinel_;
};

// Simple ring of nodes
struct Ring : List {
 public:
  Ring();
  void Push(Node* node);
  Node* Peek();
  Node* Pop();
};

}  // namespace linked

// TODO(josh): remove this
#include "util/linked.hpp"
