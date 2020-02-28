#pragma once
// Copyright 2018 Josh Bialkowski <josh.bialkowski@gmail.com

#include <cstdint>
#include <cstdlib>
#include <type_traits>

// Reference to one bit within a bitset
template <typename T>
struct BitRef {
  T* byte_block_;
  int bit_no_;

  operator bool() const {
    return (*byte_block_) & (0x01 << bit_no_);
  }

  BitRef& operator=(int value) {
    if (value) {
      (*byte_block_) |= (0x01 << bit_no_);
    } else {
      (*byte_block_) &= (~T(0) - (0x01 << bit_no_));
    }
    return *this;
  }
};

// Like std::bitset but returns assignable references
template <uint16_t N, typename T = uint32_t>
struct BitSet {
  enum {
    STORE_ELEMS_ = ((N + 8 * sizeof(T) - 1) / (8 * sizeof(T))),
    STORE_BITS_ = STORE_ELEMS_ * (sizeof(T) * 8),
    EXTRA_BITS_ = (STORE_BITS_ - N),

    // NOTE(josh): we store the data in the least significant bits
    LAST_ELEM_MASK_ = (~T(0) >> EXTRA_BITS_)
  };

  BitSet() {
    static_assert(N > 0, "BitSet<0> is not supported");
    static_assert(std::is_unsigned<T>::value,
                  "BitSet block type must be unsigned");
    static_assert(sizeof(T) > 3,
                  "BitSet block type must be 32 or 64 bit integers due to "
                  "integer promotion during bitwise operations");
  }

  T data[STORE_ELEMS_];

  void Clear() {
    for (size_t idx = 0; idx < STORE_ELEMS_; idx++) {
      data[idx] = 0;
    }
  }

  bool Any() {
    for (size_t idx = 0; idx < STORE_ELEMS_ - 1; idx++) {
      if (data[idx]) {
        return true;
      }
    }
    if (data[STORE_ELEMS_ - 1] & LAST_ELEM_MASK_) {
      return true;
    }
    return false;
  }

  bool All() {
    for (size_t idx = 0; idx < STORE_ELEMS_ - 1; idx++) {
      if (~data[idx]) {
        return false;
      }
    }
    if (~data[STORE_ELEMS_ - 1] & LAST_ELEM_MASK_) {
      return false;
    }
    return true;
  }

  bool None() {
    return !Any();
  }

  BitRef<T> operator[](int idx) {
    const int elemno = idx / (8 * sizeof(T));
    const int bitno = idx % (8 * sizeof(T));
    assert(elemno < STORE_ELEMS_);
    return BitRef<T>{&data[elemno], bitno};
  }

  const BitRef<T> operator[](int idx) const {
    const int elemno = idx / (8 * sizeof(T));
    const int bitno = idx % (8 * sizeof(T));
    assert(elemno < STORE_ELEMS_);
    return BitRef<T>{&data[elemno], bitno};
  }
};
