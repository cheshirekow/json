#pragma once
// Copyright 2018 Josh Bialkowski <josh.bialkowski@gmail.com

#include "util/linked.h"

namespace linked {

inline Node::Node() : prev(nullptr), next(nullptr) {}

inline Node::~Node() {
  Remove();
  assert(prev == nullptr || prev == this);
  assert(next == nullptr || next == this);
}

inline void Node::Remove() {
  if (prev) {
    prev->next = next;
  }
  if (next) {
    next->prev = prev;
  }
  prev = nullptr;
  next = nullptr;
}

inline void Node::InsertBefore(Node* other) {
  Remove();
  this->next = other;
  this->prev = other->prev;
  other->prev = this;
  if (this->prev) {
    this->prev->next = this;
  }
}

inline void Node::InsertAfter(Node* other) {
  Remove();
  this->prev = other;
  this->next = other->next;
  other->next = this;
  if (this->next) {
    this->next->prev = this;
  }
}

inline Node* Node::MakeRing() {
  Remove();
  prev = this;
  next = this;
  return this;
}

inline bool Iterator::operator==(const Iterator& other) const {
  return this->node == other.node;
}

inline bool Iterator::operator!=(const Iterator& other) const {
  return this->node != other.node;
}

inline Iterator& Iterator::operator++() {
  if (node) {
    node = node->next;
  }
  return *this;
}

inline Iterator Iterator::operator++() const {
  if (node) {
    return Iterator{node->next};
  } else {
    return Iterator{node};
  }
}

inline Iterator Iterator::operator+(size_t off) {
  Iterator other{node};
  for (size_t idx = 0; idx < off; ++idx) {
    ++other;
  }
  return other;
}

inline Iterator& Iterator::operator--() {
  if (node) {
    node = node->prev;
  }
  return *this;
}

inline Iterator Iterator::operator--() const {
  if (node) {
    return Iterator{node->prev};
  } else {
    return Iterator{node};
  }
}

inline Iterator Iterator::operator-(size_t off) {
  Iterator other{node};
  for (size_t idx = 0; idx < off; ++idx) {
    --other;
  }
  return other;
}

inline Node* Iterator::operator->() {
  return node;
}

inline Node* Iterator::operator*() {
  return node;
}

inline bool ConstIterator::operator==(const ConstIterator& other) const {
  return this->node == other.node;
}

inline bool ConstIterator::operator!=(const ConstIterator& other) const {
  return this->node != other.node;
}

inline ConstIterator& ConstIterator::operator++() {
  if (node) {
    node = node->next;
  }
  return *this;
}

inline ConstIterator ConstIterator::operator++() const {
  if (node) {
    return ConstIterator{node->next};
  } else {
    return ConstIterator{node};
  }
}

inline ConstIterator ConstIterator::operator+(size_t off) {
  ConstIterator other{node};
  for (size_t idx = 0; idx < off; ++idx) {
    ++other;
  }
  return other;
}

inline ConstIterator& ConstIterator::operator--() {
  if (node) {
    node = node->prev;
  }
  return *this;
}

inline ConstIterator ConstIterator::operator--() const {
  if (node) {
    return ConstIterator{node->prev};
  } else {
    return ConstIterator{node};
  }
}

inline ConstIterator ConstIterator::operator-(size_t off) {
  ConstIterator other{node};
  for (size_t idx = 0; idx < off; ++idx) {
    --other;
  }
  return other;
}

inline const Node* ConstIterator::operator->() {
  return node;
}

inline const Node* ConstIterator::operator*() {
  return node;
}

template <class ContainerOf>
inline ContainerIterator<ContainerOf>::ContainerIterator(
    Node* node, Node ContainerOf::*member)
    : member_offset_(member), node_(node) {}

template <class ContainerOf>
inline bool ContainerIterator<ContainerOf>::operator==(
    const ContainerIterator<ContainerOf>& other) const {
  return this->node_ == other.node_;
}

template <class ContainerOf>
inline bool ContainerIterator<ContainerOf>::operator!=(
    const ContainerIterator<ContainerOf>& other) const {
  return this->node_ != other.node_;
}

template <class ContainerOf>
inline ContainerIterator<ContainerOf>& ContainerIterator<ContainerOf>::
operator++() {
  if (node_) {
    node_ = node_->next;
  }
  return *this;
}

template <class ContainerOf>
inline ContainerIterator<ContainerOf> ContainerIterator<ContainerOf>::
operator++() const {
  if (node_) {
    return ContainerIterator<ContainerOf>{node_->next};
  } else {
    return ContainerIterator<ContainerOf>{node_};
  }
}
template <class ContainerOf>
inline ContainerIterator<ContainerOf> ContainerIterator<ContainerOf>::operator+(
    size_t off) {
  ContainerIterator<ContainerOf> other{node_};
  for (size_t idx = 0; idx < off; ++idx) {
    ++other;
  }
  return other;
}
template <class ContainerOf>
inline ContainerIterator<ContainerOf>& ContainerIterator<ContainerOf>::
operator--() {
  if (node_) {
    node_ = node_->prev;
  }
  return *this;
}
template <class ContainerOf>
inline ContainerIterator<ContainerOf> ContainerIterator<ContainerOf>::
operator--() const {
  if (node_) {
    return ContainerIterator<ContainerOf>{node_->prev};
  } else {
    return ContainerIterator<ContainerOf>{node_};
  }
}
template <class ContainerOf>
inline ContainerIterator<ContainerOf> ContainerIterator<ContainerOf>::operator-(
    size_t off) {
  ContainerIterator<ContainerOf> other{node_};
  for (size_t idx = 0; idx < off; ++idx) {
    --other;
  }
  return other;
}
template <class ContainerOf>
inline ContainerOf* ContainerIterator<ContainerOf>::operator->() {
  return container_of(node_, member_offset_);
}
template <class ContainerOf>
inline ContainerOf* ContainerIterator<ContainerOf>::operator*() {
  return container_of(node_, member_offset_);
}

template <class ContainerOf>
ContainerRange<ContainerOf>::ContainerRange(Node* begin, Node* end,
                                            Node ContainerOf::*member_offset)
    : begin_(begin), end_(end), member_offset_(member_offset) {}

template <class ContainerOf>
inline ContainerIterator<ContainerOf> ContainerRange<ContainerOf>::begin() {
  return ContainerIterator<ContainerOf>{begin_, member_offset_};
}

template <class ContainerOf>
inline ContainerIterator<ContainerOf> ContainerRange<ContainerOf>::end() {
  return ContainerIterator<ContainerOf>{end_, member_offset_};
}

inline List::List() {
  sentinel_.MakeRing();
}

inline Iterator List::begin() {
  return Iterator{sentinel_.next};
}

inline Iterator List::rbegin() {
  return Iterator{sentinel_.prev};
}

inline Iterator List::end() {
  return Iterator{&sentinel_};
}

inline ConstIterator List::begin() const {
  return ConstIterator{sentinel_.next};
}

inline ConstIterator List::rbegin() const {
  return ConstIterator{sentinel_.prev};
}

inline ConstIterator List::end() const {
  return ConstIterator{&sentinel_};
}

inline void List::PushBack(Node* node) {
  node->InsertBefore(&sentinel_);
}

inline void List::PushFront(Node* node) {
  node->InsertAfter(&sentinel_);
}

inline Node* List::PopFront() {
  Node* out = sentinel_.next;
  if (out == &sentinel_) {
    return nullptr;
  } else {
    out->Remove();
    return out;
  }
}

inline Node* List::PopBack() {
  Node* out = sentinel_.prev;
  if (out == &sentinel_) {
    return nullptr;
  } else {
    out->Remove();
    return out;
  }
}

inline Node* List::Front() {
  if (sentinel_.next == &sentinel_) {
    return nullptr;
  } else {
    return sentinel_.next;
  }
}

inline Node* List::Back() {
  if (sentinel_.prev == &sentinel_) {
    return nullptr;
  } else {
    return sentinel_.prev;
  }
}

inline bool List::IsEmpty() const {
  return (sentinel_.next == &sentinel_);
}

inline List::operator bool() const {
  return !IsEmpty();
}

inline void List::Clear() {
  sentinel_.MakeRing();
}

inline void List::StealNodesFrom(List* other) {
  if (other->IsEmpty()) {
    return;
  }

  Node* my_head = &sentinel_;
  Node* my_tail = sentinel_.prev;
  Node* splice_begin = other->sentinel_.next;
  Node* splice_end = other->sentinel_.prev;

  assert(my_tail);
  assert(splice_begin);
  assert(splice_end);

  other->Clear();
  my_tail->next = splice_begin;
  splice_begin->prev = my_tail;
  splice_end->next = my_head;
  my_head->prev = splice_end;
}

inline void List::GiveNodesTo(List* other) {
  other->StealNodesFrom(this);
}

template <class ContainerOf>
inline ContainerRange<ContainerOf> List::IterContainers(
    Node ContainerOf::*member_offset) {
  return ContainerRange<ContainerOf>{sentinel_.next, &sentinel_, member_offset};
}

inline Ring::Ring() {}

inline void Ring::Push(Node* node) {
  this->PushBack(node);
}

inline Node* Ring::Peek() {
  return this->Front();
}

inline Node* Ring::Pop() {
  return this->PopFront();
}

}  // namespace linked
