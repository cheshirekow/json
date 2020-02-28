=========
Changelog
=========

v0.2.1
======

* Added generic tree walk to the stream API, allows arbitrary navigation
  of json-serializable structures
* Add python script to code-generate the stream API rather than using C
  macros.
* Add utilities to escape/unescape strings for JSON serialization.
* Fix missing backslash in regex for STRING_LITERAL token
* Cleanup some compiler warnings
* Add a frontend test to execute the demo program on some canonical
  input and ensure that the lex/parse/markup output matches expected
  outputs
* ParseString will now unescape the contents
* Moved parse/emit functions to their own files
* Moved parse/emit functions out of the stream namespace
* Merge _tpl.h files into -> .h
* Make json_gen use a jinja template
* Replace remaining printf() with LOG() for parse errors

Closes: 0f7c7c3, 12fc493, 5fa508d, 71ac52e, 92c0d89, cda90c0, dc9d5bb

v0.2.0
======

Overhaul the stream API.

* Stream API no longer uses runtime pointer maps
* Implement compile time string hashing for key switch/case
* Implement new macro technique for variable number of case statements
* Emit/Parse are now implemented as overloads in json::stream::
  namespace rather than member functions of the struct. This may change
  again in the future.

v0.1.0
======

Initial commit.

* Functional low-level API for lexing/parsing JSON
* A demonstrator "stream" API for creating JSON-serializable structures
  in C++.
