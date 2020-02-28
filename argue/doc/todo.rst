====
TODO
====

2.  Add groups
3.  Add mutex groups
4.  Add argh style builders using clang-tooling which can interpret your
    function signatures and and automatically generate parsers for them.
5.  Support any unique prefix of flags the way that argparse does.
6.  Add --help-fmt option which prints help in some differnet formats for
    machine consumption. For example Markdown, ReST, json
    (i.e. generate help docs for all programs in my build).
7.  Integrate argue with JSON for a simple unified config-file / command line
    system in JSON format.
8.  Add tool to generate bash completion, and potentially arguments to assist,
    something like a global "--argue-autocomplete" on each parser. Pass
    "autocomplete=true" in the parse context.
9.  Consider adding a "IsPromotable()" interface method for actions so that
    we don't accidentally replace user-specified actions with
    ``nargs=ZERO_OR_MORE``
10. Consider using CRTRP for kwargs member classes, instead of redefining them
    in each of the specializations. Would be more compact and dedup code, but
    might get harder to debug.
11. Pass the column size into ``GetHelp()`` so that actions can implement their
    own word wrap. In particular, this will allow us to put choices on the
    first line without wrapping it with the rest of the text.
12. Add a script which updates the command help in a restructured text
    document. Use the ``$<TARGET_FILE:tgt>`` generator expression to get the
    path of the binary into the command. Use a python script which takes in
    a) the path to the binary, b) the binary name, c) the path to documentation
    file to update. Use some kind of sentinel comment to indicate where to
    paste.
13. Add storage models and DestinationField overloads for fixed-sized arrays,
    sets, and maps
14. Add tests and cmake tooling to execute programs with "--help", and
    "--version" to ensure help text doesn't change unexpectedly.
