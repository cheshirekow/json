=========
Changelog
=========

-----------
v0.1 series
-----------

v0.1.2 -- in progress
=====================

* Add support for ``--version`` and ``--help`` without the corresponding short
  flag (i.e. no ``-v`` or ``-h``)
* Add macro shims to work with gcc which doesn't support designated
  initializers
* Add support for ``nargs=REMAINDER``
* Added ``argue``/``glog`` integration via a function to add command line
  flags for all the ``glog`` ``gflag`` globals
* Did some build system cleanup
* Removed individual exception classes, unifying them into a single one
* Replace hacky assertion stream with ``fmt::format()`` usages.
* Replace KWargs class with optional containers with KWargs field objects
  that pass-through to setters instead.
* Don't latch help text at help tree construction time, instead query help
  out of the action objects at runtime. This is so that subparsers know what
  children they have and can generate choices text.

v0.1.1
======

* Implemented subparser support

v0.1.0
======

Initial release! This is mostly a proof of concept initial implementation. It
lacks several features I would consider required but works pretty well to start
using it.

Features:

* argparse-like syntax
* type-safe implementations for ``store``, ``store_const``, ``help``, and
  ``version`` actions
* support for scalar ``(nargs=<default>, nargs='?')`` or
  container (nargs=<n>, nargs='+', nargs='*') assignment
* provides different exception types and error messages for exceptional
  conditions so they can be distinguised between input errors (user errors),
  library usage errors (your bugs), or library errors (my bugs).
* support for custom actions
* output formatting for usage, version, and full help text
