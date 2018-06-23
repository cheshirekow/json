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