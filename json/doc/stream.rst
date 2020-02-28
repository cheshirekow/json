==========
Stream API
==========

.. default-domain:: cpp

The "stream" API is meant to provide a mechanism for creating high level
bindings for parsing a JSON character buffer directly into static objects.
You can use it to make your C/C++ structures JSON-serializable.

The JSON event stream is provided by the :class:`LexerParser` which maintains
a state machine for the tokenization and semantic parsing of a JSON text
document. It provides the :function:`GetNextEvent` which sequentially returns
each semantic event in the stream.

-------
Parsing
-------

Parsing JSON entities into C++ entities is generally done through overloads of
the :func:`ParseValue` function, which has the following signature:

.. code:: cpp

    void ParseValue(const Event& event, LexerParser* stream, ValueType* value);


for each parseable :expr:`ValueType`. :func:`ParseValue` is essentially a
dispatcher which dispatches either a token parser (for scalar types), an object
parser (for object types) or an array parser (for array types).

For scalar types, the implementation
generally looks something like this, where it dispatches a token parser:

.. code:: cpp

    void ParseValue(const Event& event, LexerParser* stream, Foo* value) {
      if (event.typeno != json::Event::VALUE_LITERAL) {
        LOG(WARNING) << fmt::format("Cannot parse {} as value at {}:{}",
                                    json::Event::ToString(event.typeno),
                                    event.token.location.lineno,
                                    event.token.location.colno);
      }
      ParseToken(event.token, value);
    }

For C/C++ structures and classes, the implementation generally looks like
this:

.. code:: cpp

   void ParseValue(const Event& event, LexerParser* stream, Foo* out) {
     ParseObject(event, stream, out);
   }

:func:`ParseObject` is a function template which simply does the following:

1. Ensures that the parse stream matches the expected sequence:

   a. Starting with :expr:`OBJECT_BEGIN`
   b. Followed by a sequence of :expr:`(OBJECT_KEY, value)` pairs
   c. Ending with :expr:`OBJECT_END`

2. Iterates through each of the :expr:`(OBJECT_KEY, value)` pairs and
   calls :func:`ParseField` on each pair.

:func:`ParseField` is, in turn, overloaded for every serializable type. The
purpose of :func:`ParseField` is just to select which member to call
:func:`ParseValue` on, given the current JSON object key. The
implementation generally looks like this:

.. code:: cpp

    int ParseField(const re2::StringPiece& key, const Event& event,
                   LexerParser* stream, Foo* out) {
      uint64_t keyid = RuntimeHash(key);
      switch (keyid) {
        case Hash("field_a"):
          ParseValue(event, stream, &out->field_a);
          break;
        case Hash("field_b"):
          ParseValue(event, stream, &out->field_b);
          break;
        case Hash("field_c"):
          ParseValue(event, stream, &out->field_c);
          break;
        default:
          SinkValue(event, stream);
          return 1;
      }
      return 0;
    }

In summary, the logic follows this call-map:

.. code:: text

   ParseValue
    ├─ ParseToken
    └─ ParseObject (template)
       └─ ParseField (overload)
          └─ ParseValue (overload)

All JSON-parsable types must implement :func:`ParseValue`. JSON-parsable
scalars my utilize :func:`ParseToken` if an overload exists, or they may
implement the token parser directly in :func:`ParseValue`. JSON-parsable objects
must implement :func:`ParseValue` as a single-line function call to
:func:`ParseObject` and must also implement :func:`ParseField`.

Note that :func:`ParseValue` overloads are necessary mostly due to the static
nature of C++. You could imagine an implementation that looks like the
following:

.. code:: cpp

   void ParseValue(const Event& event, LexerParser* stream, Foo* value) {
     if (event.typeno == json::Event::BEGIN_OBJECT) {
       ParseObject(event, stream, value);
     } elif (event.typeno == json::Event::BEGIN_ARRAY) {
       ParseArray(event, stream, value);
     } else {
       ParseScalar(event.token, value);
     }
   }

But for a given type :class:`Foo` only one of these functions will have
applicable overloads. The other two wont exist and we'll get compiler errors.
This is why :func:`ParseValue` needs to be overloaded for every type.


-------------
Type Registry
-------------

.. TODO(josh): Add explaination of issues with two-stage lookup and name
   resolution. See the header comment in registry_poc.cc

Each JSON serializable type should register itself with the marshalling
registry. This should be done at global scope with an assignment like
so:

.. code:: cpp

   // Register a scalar serializable type
   // The ParseValue implementation will refer to an instanciation of the
   // ParseAsToken template.
   static const int _dummy0 = json::registry::add_scalar<T>(token_parse_fun);

   // Register an object type:
   // The ParseValue implementation will refer to an instanciation of the
   // ParseAsObject template, and the input parameter is the pointer to the
   // ParseField implementation.
   static const int _dummy1 = json::registry::add_scalar<T>(parse_field_fun);

Alternatively they can be registered in an :code:`__attribute__(constructor)`
function.

We modify the API for :func:`parse_field` to take in a pointer to a registry
object, and dispatch the :func:`parse_value` member of the registry instead.

.. code:: cpp

    int parse_field(const Registry& registry,
                    const re2::StringPiece& key, const Event& event,
                    LexerParser* stream, Foo* out) {
      uint64_t keyid = RuntimeHash(key);
      switch (keyid) {
        case Hash("field_a"):
          registry.parse_value(event, stream, &out->field_a);
          break;
        case Hash("field_b"):
          registry.parse_value(event, stream, &out->field_b);
          break;
        case Hash("field_c"):
          registry.parse_value(event, stream, &out->field_c);
          break;
        default:
          SinkValue(event, stream);
          return 1;
      }
      return 0;
    }
