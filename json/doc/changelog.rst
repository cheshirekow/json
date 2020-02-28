=========
Changelog
=========

v0.2.5
======

* Tokenizer and Parser now have peek-ahead capability
* Don't choke on empty list ("[]") or object ("{}")
* Fix sink_object and sink_list were inconsistent in whether or not they
  expected the opening event to have been consumed or not
* Add parse utilities for parsing lists into standard containers with
  push_back members
* Add parse API for lists mirroring the parse API for objects
* Fix `sink_object` sign error in loop condition
* Add string type name to the registry entry
* Add stream specialization for `shared_ptr`

v0.2.4
======

* change all functions to GNU style snake_case

v0.2.3
======

* get rid of WalkOpts
* remove registry_poc.cc
* get rid of BufPrinter and FmtError, replace with static stringstream
* make Registry* const within the dumper
* parse_array -> parse_list
* make order of parameters consistent in type_registry.h API
* parse functions all return integers
* some cleanup/reorg of type_registry.h/cc


v0.2.2
======

* Use SerializeOpts inside StreamDumper to pretty-print (ish)
* convert json_gen to generate code for the new registry-based
  streaming API and update stream_gen_test to use it
* change stream_macros.h to generate code for the new registry-based
  streaming API and update stream_test to use it.
* add kCompactOpts to json.h/cc (for unpretty-printing)
* add documentation for the new streaming API and some general docs
* delete old streaming API

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
