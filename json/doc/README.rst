=====
jjson
=====

A C++ library for working with JavaScript Object Notation.

-------------
Low-level API
-------------

There is a straight-forward, low level API that you may find suitable for
working with JSON documents. It follows a typical lexer / parser pattern. You
can manage the lex and parse steps separately with ``json::Scanner`` and
``json::Parser``, or you can you use the thin combination ``json::LexerParser``.

To work with the parse stream from a text document:

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

To work with a token and event stream directly from a text document:

.. code-block:: c++

    int DoTokenStream(const std::string& content, std::ostream* log) {
      json::Scanner scanner{};
      json::Parser  parser{};
      json::Error{};

      int status = scanner.Init(&error);
      if(status < 0){
        (*log) << json::Error::ToString(error.code) << ": " << error.msg;
        return -1;
      }

      scanner.Begin(content);

      json::Token token{};
      json::Event event{};
      while(scanner.Pump(&token, &error) == 0){
        int status = parser.HandleToken(token, &event, &error);
        // error
        if(status < 0){
          (*log) << json::Error::ToString(error.code) << ": " << error.msg;
          return -1;
        } else if(status > 0){
          // An actionable event has occured, do something with the event
        } else {
          // No actionable event, but you can do something with the token
          // if you want. This means the token is either whitespace, colon,
          // or comma.
        }
      }

      if(error.code != json::Error::LEX_INPUT_FINISHED){
        return -1;
      } else {
        return 0;
      }
    }

---------------
High Level APIs
---------------

There are a couple of experimental high level APIs that you might find
useful. These should not be considered production ready and are likely to
change int he future as I continue to experiment with different implementations.

The Stream API
==============

The stream API allows you to construct JSON-serializable native structures. To
use the API include `stream.h` and `stream_macros.h` in any header where you
declare your structures, and use the `JSON_STREAM` macro to expose fields.

For example:

.. code-block:: c++

    #include <fstream>
    #include <iostream>
    #include "json/stream.h"
    #include "json/stream_macros.h"


    struct MyStruct {
      struct {
        int a = 1;
        double b = 3.14;
        float e = 1.2;
        int f = 3;
        JSON_STREAM(a, b, e, f);
      } foo;

      struct {
        int c = 2;
        float d = 3.2f;
        JSON_STREAM(c, d);
      } bar;

      struct {
        int a = 1;
        float b = 2.0;
        JSON_STREAM(a, b);
      } boz[2];

      JSON_STREAM(foo, bar, boz);
    };

    int main(int argc, char** argv){
      // argv[1] is the name of a JSON file to read in
      std::string content;
      content.reserve(1024 * 1024);
      content.assign((std::istreambuf_iterator<char>(argv[1])),
                      std::istreambuf_iterator<char>());

      MyStruct obj;

      // Parse the input file and assign fields of MyStruct
      json::stream::Parse(content, &obj);

      // Serialize the resulting structure into a char buffer
      content.resize(512, '\n');
      json::stream::Emit(obj, {.indent=2, .separators={": ", ","}}, &content[0],
                        &content[512]);

      // and print to stdout
      std::cout << content;
    }

----------------
The json program
----------------

Included in the package is a simple json utility application intended to
demonstrate usage of the library. The command `json` can dump the lex'ed token
stream or the parsed event stream. It can also validate a json file or markup
its contents with html that can be used to publish semantic-highlighted
json documents.

::

    ./json/json
    ===========
    version: 0.0.1
    author : Josh Bialkowski <josh.bialkowski@gmail.com>
    copyright: (C) 2018

    ./json/json [-h/--help] [-v/--version] <command>

    Demonstrates the usage of the jjson library to lex and parse JSON data

    Flags:
    ------
    -h  --help          print this help message
    -v  --version       print version information and exit

    Positionals:
    ------------
    command             see subcommands

    Subcommands:
    ============

    Subcommand `lex`
    lex [-h/--help] [infile]

    Lex the file and dump token information
    ----
    -h  --help          print this help message
    ----
    infile              Path to input, '-' for stdin

    Subcommand `markup`
    markup [-h/--help] [-o/--omit-template] [infile]

    Parse and dump the contents with HTML markup
    ----
    -h  --help          print this help message
    -o  --omit-template output just the content
    ----
    infile              Path to input, '-' for stdin

    Subcommand `parse`
    parse [-h/--help] [infile]

    Parse the file and dump actionable parse events
    ----
    -h  --help          print this help message
    ----
    infile              Path to input, '-' for stdin

    Subcommand `verify`
    verify [-h/--help] [infile]

    Parse the file and exit with 0 if it's valid json
    ----
    -h  --help          print this help message
    ----
    infile              Path to input, '-' for stdin
