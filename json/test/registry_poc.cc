// Copyright 2018 Josh Bialkowski <josh.bialkowski@gmail.com>
// Proof-of-concept/prototype for a typesafe overload registry to work around
// issues of name resolution and two-phase lookup.
/**
 * The technique demonstrated in this file can possibly be used to work-around
 * issues with name resolution and two-phase lookup. The problem is that if
 * we want a function like
 *
 * ```
 * template <typename T>
 * void EmitField(const char* key, const T& value, const SerializeOpts& opts,
 *                size_t depth, BufPrinter* out) {
 *   FmtIndent(out, opts.indent, depth + 1);
 *   (*out)("\"%s\"%s", key, opts.separators[0]);
 *   EmitValue(value, opts, depth + 1, out);
 * }
 * ```
 *
 * which calls ``EmitValue`` and we expect ``EmitValue`` to have overloads
 * for every type T that we might encounter in a type-hierarchy. Then the
 * template definition (shown above) must exist *after* the declaration of
 * every such overload. This leads for a kind of poor user experience because
 * users of the library must include a silly "stream_tpl.h" header (containing
 * the above implementation) *after* they include all the headers with their
 * overloads. Furthermore, this header now becomes order-dependant meaning
 * the library user puts it in the main source of any translation unit where
 * it's neeeded (at the "appropriate" place), or they are constantly debugging
 * weird header-include-order issues.
 *
 * The technique below get's around this by storing a *runtime* map of all
 * overload function pointers that are available. The map is keyed on the
 * address of an unrelated dummy_function whose template is declared in the
 * same header as the registry.
 *
 * The technique is not fool proof. It introduces some issues of initialization
 * order. The registry is populated in an indeterminate order based on whatever
 * order the compiler/linker decides for the global initializations that
 * call the registration functions. As a result, we cannot reliably query the
 * the registry prior to main() entry. We might want to add some sanity
 * check mechanism to prevent accidental query before this point in time.
 */

#include <iostream>
#include <map>

template <class T>
void dummy_function(T* arg) {
  return;
}

// Store, at runtime, overloads of a function for various types T
class OverloadRegistry {
 public:
  template <typename T>
  int register_overload(void (*overload)(const T&)) {
    void (*dummy_instance)(T * arg) = &dummy_function<T>;
    void* key = reinterpret_cast<void*>(dummy_instance);
    void* value = reinterpret_cast<void*>(overload);
    registry_[key] = value;
    return 0;
  }

  template <typename T>
  void (*get_overload())(const T& value) {  // NOLINT
    typedef void(overload_type)(const T&);
    void (*dummy_instance)(T * arg) = &dummy_function<T>;
    void* key = reinterpret_cast<void*>(dummy_instance);
    auto iter = registry_.find(key);
    void* value = nullptr;
    if (iter != registry_.end()) {
      value = iter->second;
    }
    return reinterpret_cast<overload_type*>(value);
  }

 private:
  std::map<void*, void*> registry_;
};

OverloadRegistry* get_registry() {
  static OverloadRegistry* g_overload_registry_ = new OverloadRegistry{};
  return g_overload_registry_;
}

// overload_double.cc
void overloaded_function(const double& value) {
  std::cout << "Overload for double\n";
}

const int _dummy0 =
    get_registry()->register_overload<double>(overloaded_function);

// overload_float.cc
void overloaded_function(const float& value) {
  std::cout << "Overload for float\n";
}

const int _dummy1 =
    get_registry()->register_overload<float>(overloaded_function);

// overload_int.cc
void overloaded_function(const int& value) {
  std::cout << "overload for int\n";
}

const int _dummy2 = get_registry()->register_overload<int>(overloaded_function);

int main(int argc, char** argv) {
  std::cout << "here" << std::endl;
  get_registry()->get_overload<double>()(1.0);
  get_registry()->get_overload<float>()(1.0f);
  get_registry()->get_overload<int>()(1);
  std::cout << std::endl;
  exit(0);
}
