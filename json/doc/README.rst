====
json
====

A C++ library for working with JavaScript Object Notation.

The design of this library makes it especialy suitable for use in embedded
applications. In particular, the design supports:

1. Parse into static structures with no memory allocations
2. Serialize into fixed output buffers with no memory allocations
3. No dependency on the standard library

.. warning::

   While the design of the library enables these features, the current
   implementation doesn't quite meet these goals. For the most part there's
   a few standard library uses that need to be replaced with fixed-memory
   data structures and some of the standard library support is written in the
   same files as the core library. These will be segregated into their own
   files soon.

-------------
Low-level API
-------------

There is a straight-forward, low level API that you may find suitable for
working with JSON documents. For example, a simple program that responds to
all of the semantic elements in a JSON document:

.. code-block:: c++

    int DoParseStream(const std::string& content, std::ostream* log) {
      json::LexerScanner stream{};
      json::Error{};

      int status = stream.Init(&error);
      if(status < 0){
        (*log) << json::Error::ToString(error.code) << ": " << error.msg;
        return status;
      }

      stream.Begin(content);

      json::Event event{}
      while(stream.GetNextEvent(&event, &error) == 0){
        switch(event.typeno){
          // Emitted on the start of a json object (i.e. the '{' token)
          case json::Event::OBJECT_BEGIN:

          // Emitted when a key is parsed. The event.token.spelling contains the
          // content of the key as a string literal (i.e. including the double
          // quotes)
          case json::Event::OBJECT_KEY:

          // Emitted on the end of a json object (i.e. the '}' token)
          case json::Event::OBJECT_END:

          // Emitted on the start of a json list (i.e. the '[' token)
          case json::Event::LIST_BEGIN:

          // Emitted on the end of a json list (i.e. the ']' token)
          case json::Event::LIST_END:

          // Emitted on any value literal including numeric, string, null, or
          // boolean.
          case json::Event::VALUE_LITERAL:
            break;
        }
      }

      if(error.code != json::Error::LEX_INPUT_FINISHED){
        (*log) << json::Error::ToString(error.code) << ": " << error.msg;
        return -1;
      } else {
        return 0;
      }
    }

---------------
High Level APIs
---------------

There are a couple of experimental high level APIs that you might find
useful.

The Stream API
==============

The stream API allows you to construct JSON-serializable native structures.
There are magic macros and a code-generator that can simplify the process of
generating the bindings.

For example:

.. code-block:: c++

    #include <fstream>
    #include <iostream>
    #include "json/json.h"
    #include "json/stream_macros.h"


    struct MyStruct {
      struct {
        int a = 1;
        double b = 3.14;
        float e = 1.2;
        int f = 3;
      } foo;

      struct {
        int c = 2;
        float d = 3.2f;
      } bar;

      struct {
        int a = 1;
        float b = 2.0;
      } boz[2];
    };

    JSON_DEFN(MyStruct, foo, bar, boz);
    JSON_DEFN2(decltype(MyStruct::foo), FOO, a, b, e, f);
    JSON_DEFN2(decltype(MyStruct::bar), BAR, c, d);
    JSON_DEFN2(decltype(MyStruct::boz), BOZ, a, b);
    JSON_REGISTER_GLOBALLY(X, MyStruct, FOO, BAR, BOZ);

    int main(int argc, char** argv){
      // argv[1] is the name of a JSON file to read in
      std::string content;
      content.reserve(1024 * 1024);
      content.assign((std::istreambuf_iterator<char>(argv[1])),
                      std::istreambuf_iterator<char>());
      MyStruct obj;

      // Parse the input file and assign fields of MyStruct
      json::stream::parse(content, &obj);

      // Serialize the resulting structure into a string and write to
      // stdout
      std::cout << json::stream::dump(obj);
    }

The Builder API
===============

The Builder API can generate `Variant` trees that serializable to json. For
example:

.. code:: c++

    #include "json/builder.h"
    using namespace json;  // NOLINT
    using namespace json::insource;  // NOLINT

    int main(int argc, char* argv) {

      Variant tree =                   //
        json::Build(O{"hello", 123, "world",
                      O{"foo", O{
                                   "far", 123,      //
                                   "fuz", "hello",  //
                                   "fur", 42.7e2,   //
                                   "fox", true,     //
                                   "fut", false,    //
                                   "fit", nullptr   //
                               }}});
      std::string buf;
      buf.resize(256);
      size_t realsize = tree.serialize(&buf[0], &buf.back());
      buf.resize(realsize);
      std::cout << buf;
    }

For more information, see `builder.h`.

----------------
The json program
----------------

Included in the package is a simple json utility application intended to
demonstrate usage of the library. The command `json` can dump the lex'ed token
stream or the parsed event stream. It can also validate a json file or markup
its contents with html that can be used to publish semantic-highlighted
json documents.

::

    ====
    json
    ====
    version: 0.2.0
    author : Josh Bialkowski <josh.bialkowski@gmail.com>
    copyright: (C) 2018

    json [-h/--help] [-v/--version] <command>

    Demonstrates the usage of the json library to lex and parse JSON data

    Flags:
    ------
    -h  --help          print this help message
    -v  --version       print version information and exit

    Positionals:
    ------------
    command             Each subcommand has it's own options and arguments, see
                        individual subcommand help.

    Subcommands:
    ------------
    lex                 Lex the file and dump token information
    markup              Parse and dump the contents with HTML markup
    parse               Parse the file and dump actionable parse events
    verify              Parse the file and exit with 0 if it's valid json
