====
TODO
====

* Cleanup init API, to be more RAII-ish
* Implement Variant parser
* Cleanup ItemParser to be a little more useful

* Add library options structure to each of the functions. Some ideas for things
  that might show up in there are:

    1. allow comments
    2. log level
    3. warn message callback

* Add parser statistics output:

    1. How many of each type of json thing were parsed (object, list, int, etc)
    2. Bytes required to copy out all of the parsed strings.

* Use a fixed stack size for parse stack?
* Implement range iterators in pipeline.[h|cc]
* Move builder stuff into variant header/cc

==================
Stream API Walking
==================

Current streaming API sucks for walking. It's very complicated, and include
order matters which is pretty shitty. Need to figure out a way to improve
this. Idea: instead of overloading WalkValue for all types, just provide an
overload WalkObject() for all the stream types. Then, leave it up to the Walker
object to actually dispatch. In other words remove WalkValue and all it's
overloads.

Modify the `Walker` api so that it has the following methods::

    // previously ConsumeEvent
    Walker::Notify(const WalkEvent& event);

    // Consume the value. Specify in `out` whether or not to recurse on the
    // object or list if `T` using the global dispatch system. If no valid
    // overload of this function can be found, the global dispatch system will
    // be used. If no global dispatch overload is found, a warning will be issued.
    Walker::ConsumeValue(const WalkOpts& opts, T* value, WalkOut* out);
