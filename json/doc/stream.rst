==========
Stream API
==========

.. default-domain:: cpp
.. cpp:namespace:: json

The "stream" API is meant to provide a mechanism for creating high level
bindings for parsing a JSON character buffer directly into static objects, and
for dumping statically defined structures out to JSON. You can use it to make
your C/C++ structures JSON-serializable.

-------
Parsing
-------

The JSON event stream is provided by the :class:`LexerParser` which maintains
a state machine for the tokenization and semantic parsing of a JSON text
document. It provides the :func:`GetNextEvent` which sequentially returns
each semantic event in the stream.

Parsing is managed by a :class:`stream::Registry` object, which acts as a
kind of plugin registry storing pointers to parse functions. There are two
types of parsers that are stored in the registry:

1. Field parsers: which basically just encode a list of fields for a
   particular type and can dispatch a typesafe parse for each.
2. Scalar parsers: which know how to turn a string into value of some
   particular type.

The library includes Scalar parsers for all the built-in numeric types and
some string types. The field parsers are usually what you want to add. A
field parser for a type :class:`T` is a function with the signature:

.. code:: c++

    int parse_T(
        const stream::Registry& registry, const re2::StringPiece& fieldname,
        LexerParser* event_stream, T* value);

The implementation of a field parser is pretty standard, just providing a
dispatch for each field name and usually lookes this:

.. code:: c++

    int parse_T(
        const json::stream::Registry& registry,
        const re2::StringPiece& fieldname,
        json::LexerParser* event_stream, T* value){
      int fieldno = json::RuntimeHash(fieldname);
      switch(fieldno){
        case json::Hash("foo"):
          return registry.parse_value(event_stream, &value->foo);
        case json::Hash("bar"):
          return registry.parse_value(event_stream, &value->bar);
        default:
          json::sink_value(event_stream);
          break;
      }
      return 1;
    }

In fact the above function definition can be generated automatically using
the macro :c:macro:`JSON_DEFN(T, foo, bar);`, or by utilizing the ``json_gen``
code generator.

For scalar types, the signature is:

.. code:: c++

    void parse_T(const json::Token& token, T* value);

Scalar parsers taking in a :class:`json::Token` generally look at the
`type` and `spelling` fields to parse the token into a scalar value.
Most of the json native types map easily to native C++ types and a
`Registry` already includes scalar parsers for these types (so you don't have
to write them).


-------
Dumping
-------

Outputting JSON is done through a `stream::Dumper` object. A `Dumper` knows
how to write out all of the native C++ numeric types and several string types.
For object types you must provide a field-dumper, which is a function with
the signature:

.. code:: c++

   int dumpfields_T(const T& value, Dumper* dumper);

The implementation, like `parsefields` is usually pretty standard and looks
something like this:

.. code:: c++

   int dumpfields_T(const T& value, Dumper* dumper){
      int result = 0;
      result |= dumper->dump_field("foo", value->foo);
      result |= dumper->dump_field("bar", value->bar);
      result |= dumper->dump_field("baz", value->baz);
      return result;
   }

In fact the above function definition can be generated automatically using
the macro :c:macro:`JSON_DEFN(T, foo, bar);`, or by utilizing the ``json_gen``
code generator.

For scalar types, the signature is the same, but the implementation usually
looks something like this:

.. code:: c++

    int dumpscalar_U(const U& value, Dumper* dumper){
      dumper->dump_primitive(static_cast<double>(value));
    }

The `Dumper` interface implements `dump_primitive` for all of the built-in
c++ native numerical types and several common string types. For custom types,
you will need to implement a conversion to one of these types.

-------------------
The Global Registry
-------------------

There is a global registry provided for convenience available through
`json::stream::global_registry()`. You can take advantage of static
initialization to populate the registry with your custom types. For example:

.. code:: c++

    static int register_globals_ABC(json::Registry* registry){
      registry->register_object(
          parsefields_Foo, dumpfields_Foo);
      registry->register_object(
          parsefields_Bar, dumpfields_Bar);
      registry->register_object(
          parsefields_Baz, dumpfields_Baz);
      return 0;
    }

    static const int kDummy_ABC = register_globals_ABC(
      json::stream::global_registry());

In fact, this is exactly what is done by the macro invocation
:c:macro:`JSON_REGISTER_GLOBALLY(ABC, Foo, Bar, Baz);`. If you want a macro
to generate the function definition but not register it globally during
static initialization, you can use the macro invocation
:c:macro:`JSON_DEFN_REGISTRATION_FN(ABC, Foo, Bar, Baz;`. Both of these
macros can be prefixed by `static` to make the corresponding function
static.

--------------------------
Using the json_gen program
--------------------------

In addition to C-Preprocessor macros to generate parsers, dumpers and
registration functions, there is a python program `json_gen.py` that can
generate them for you. The usage is:

.. code::

    usage: json_gen.py [-h] [-o OUTDIR] [-b BASENAME] infiles [infiles ...]

    Generate headers/sources for json-serializable streaming interface. This does
    the same thing as the JSON_DECL and JSON_DEFN macros but allows for more
    readable debugging as the source code isn't hidden beneath the preprocessor
    macros.

    positional arguments:
      infiles               specification files to process

    optional arguments:
      -h, --help            show this help message and exit
      -o OUTDIR, --outdir OUTDIR
                            directory where to put output files. Default is cwd
      -b BASENAME, --basename BASENAME
                            basename of output files. Default is basename of input
                            file.

The input files are written in python and are declarative in nature. There
are only a couple of functions that are needed:


.. code:: py

   def set_options(
      include_global_registration=True,
      emit_static_functions=False,
      namespaces=None,
      registration_suffix=None):
   """
   Set additional options:
    * include_global_registration: if true, the custom type will be
      registered with the global registry during static initialization
    * emit_static_functions: if true, a header file is not generated and
      the parse/dump and registration functions will be declared static
    * namespaces: a list of namespaces under which the generated functions
      should be named.
   """

   def add_header_includes(list_of_headers):
   """
   Add to the list of headers to include in the generated header file.
   """

   def add_source_includes(list_of_includes):
   """
   Add to the list of headers to include in the generated source file.
   """

   def decl_json(spec):
   """
   <spec> is a dictionary where keys are type declarations
   (e.g. "Foo", "foo::Bar::Baz", "decltype(FooBar::z)") which map to
   a list of strings that denote the fields which should be serialized for
   that type.

   It will generate a pair of functions parsefields_<suffix> and
   dumpfields_<suffix> implementing the parse/dump API for a custom type
   specified by <decl>. <suffix> is generated from the string <decl>.
   """

For example, consider the following:

.. code:: c++

   // file: foo.h
   #pragma once
   struct Bar {
      int field_c;
      std::string field_d;
   };

   struct Foo {
      int field_a;
      Bar field_b;
   };

Then we might create the following specification file:

.. code:: py

   # file: json_gen_foo.py
   set_options(registration_suffix="Foo")
   add_header_includes(["foo.h"])
   decl_json({
    "Foo": ["field_a", "field_b"],
    "Bar": ["field_c", "field_d"]})

And we would execute:

.. code:: text

   $ python /path/to/json/json_gen.py json_gen_foo.py

Which would generate two files:

.. code:: c++

   // json_gen_foo.h
   # pragma once
   #include "json/json.h"
   #include "json/type_registry.h"
   #include "foo.h"

   int parsefields_Foo(
       const json::stream::Registry& registry, const re2::StringPiece& key,
       json::LexerParser* stream, Foo* out);
   int dumpfields_Foo(
       const Foo& value, json::Dumper* dumper);

   int parsefields_Bar(
       const json::stream::Registry& registry, const re2::StringPiece& key,
       json::LexerParser* stream, Bar* out);
   int dumpfields_Bar(
       const Bar& value, json::Dumper* dumper);

.. code:: c++

    // json_gen_foo.cc
    #include "json_gen_foo.h"

    int parsefields_Foo(
        const json::stream::Registry& registry, const re2::StringPiece& key,
        json::LexerParser* stream, Foo* out){
      uint64_t keyid = json::RuntimeHash(key);
      switch(keyid){
        case json::Hash("field_a"):
          return registry.parse_value(stream, &out->field_a);
        case json::Hash("field_b"):
          return registry.parse_value(stream, &out->field_b);
        default:
          json::sink_value(stream);
          return 1;
      }
      return 0;
    }

    int dumpfields_Foo(
        const Foo& value, json::Dumper* dumper){
      int result = 0;
      result |= dumper->dump_field("field_a", value.field_a);
      result |= dumper->dump_field("field_b", value.field_b);
      return result;
    }

    int parsefields_Bar(
        const json::stream::Registry& registry, const re2::StringPiece& key,
        json::LexerParser* stream, Bar* out){
      uint64_t keyid = json::RuntimeHash(key);
      switch(keyid){
        case json::Hash("field_c"):
          return registry.parse_value(stream, &out->field_c);
        case json::Hash("field_d"):
          return registry.parse_value(stream, &out->field_d);
        default:
          json::sink_value(stream);
          return 1;
      }
      return 0;
    }

    int dumpfields_Bar(
        const Bar& value, json::Dumper* dumper){
      int result = 0;
      result |= dumper->dump_field("field_c", value.field_c);
      result |= dumper->dump_field("field_d", value.field_d);
      return result;
    }

    int register_types_Foo(json::Registry* registry){
      registry->register_object(
          parsefields_Foo, dumpfields_Foo);
      registry->register_object(
          parsefields_Bar, dumpfields_Bar);
      return 0;
    }

    static const int kDummy_Foo = register_types_Foo(json::global_registry());

----------------------
Implementation Details
----------------------

The type registry allows us to store function pointers to each of the
conversion functions for a specific type. The parse and dump implementations
are based on function templates to do most of the work, but the field
enumerators and tokenization functions are looked up at runtime.

This is done by associating a unique numeric key with each type that is
registered in the type registry. The key is actually the address of a
template function instanciation (:func:`json::stream::sentinel_function<T>`).
The function has an empty body and exists just to help the program generate
unique keys for each type.

The type keys are determined when the program is linked (when the function
address is determined). The parse/dump implementation functions are added
to the registry during static initialization (the function pointers are
stored in a map using the type key). Then during the parse/dump dispatch
they are retrieved from the registry using the same key.

The implementation is in `type_registry.h` and `type_registry.cc`.
