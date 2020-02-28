====
JSON
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

.. toctree::
   :maxdepth: 2
   :caption: Contents:

   low_level
   stream
   old_stream
   json_program
   changelog
   todo

Indices and tables
==================

* :ref:`genindex`
* :ref:`modindex`
* :ref:`search`
