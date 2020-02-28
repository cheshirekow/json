#pragma once
// Copyright 2018 Josh Bialkowski <josh.bialkowski@gmail.com>

#include <string>

namespace std {

template <bool B, class T, class F>
using conditional_t = typename conditional<B, T, F>::type;

}  // namespace std

namespace argue {

template <typename T, typename _ = void>
struct is_container : std::false_type {};

template <typename... Ts>
struct is_container_helper {};

// Metaprogram resolves true if T is a container-like class, or false if it
// is not.
template <typename T>
struct is_container<
    T, std::conditional_t<
           false,
           is_container_helper<typename T::value_type, typename T::size_type,
                               typename T::allocator_type, typename T::iterator,
                               typename T::const_iterator,
                               decltype(std::declval<T>().size()),
                               decltype(std::declval<T>().begin()),
                               decltype(std::declval<T>().end()),
                               decltype(std::declval<T>().cbegin()),
                               decltype(std::declval<T>().cend())>,
           void>> : public std::true_type {};

template <>
struct is_container<std::string, void> : std::false_type {};

inline void StringTest() {
  static_assert(!is_container<std::string>::value,
                "is_container<std::string> should be false");
}

template <typename T, bool is_container>
struct get_choice_type_helper {
  typedef T type;
};

template <typename T>
struct get_choice_type_helper<T, true> {
  typedef typename T::value_type type;
};

// Metaprogram yields the value type of T if it is a container, or T itself
// if it is not a container.
template <typename T>
struct get_choice_type {
  typedef typename get_choice_type_helper<T, is_container<T>::value>::type type;
};

// Sentinel type used as a placeholder in templates when the type doesn't
// matter, or as a value in Optional when assignment is meant to clear the
// storage.
struct NoneType {};
extern const NoneType kNone;
std::ostream& operator<<(std::ostream& out, NoneType none);

// NoneType isn't really ordered, but we want them to be storable in a set
// or other needs-ordering usecases.
bool operator<(const NoneType& a, const NoneType& b);

// TODO(josh): move this to cc
inline bool operator==(const NoneType& a, const NoneType& b) {
  return true;
}

int Parse(const std::string& str, NoneType* dummy);

std::ostream& operator<<(std::ostream& out, NoneType none) {
  out << "<None>";
  return out;
}

const NoneType kNone{};

bool operator<(const NoneType& a, const NoneType& b) {
  return false;
}

int Parse(const std::string& str, NoneType* dummy) {
  return -1;
}

}  // namespace argue