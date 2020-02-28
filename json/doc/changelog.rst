=========
Changelog
=========

v0.2.0
======

Overhaul the stream API.

* Stream API no longer uses runtime pointer maps
* Implement compile time string hashing for key switch/case
* Implement new macro technique for variable number of case statements
* Emit/Parse are now implemented as overloads in json::stream:: namespace
  rather than member functions of the struct. This may change again in the
  future.

v0.1.0
======

Initial commit.

* Functional low-level API for lexing/parsing JSON
* A demonstrator "stream" API for creating JSON-serializable structures
  in C++.
