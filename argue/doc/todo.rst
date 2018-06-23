====
TODO
====

2.  Add groups
3.  Add mutex groups
5.  Add argh style builders using clang-tooling which can interpret your
    function signatures and and automatically generate parsers for them.
6.  Support any unique prefix of flags the way that argparse does.
7.  Make the help output prettier, especially with subcommands.

    * Don't recurse more than one level with subcommands
    * Don't print as much decoration (i.e. all the rulers) for subcommand help
      as for the main help.
    * Use ReST or Markdown style headings for different sections.
    * Make output valid ReST or Markdown

8.  Add --help-fmt option which prints help in some differnet formats for
    machine consumption. For example Markdown, ReST, json
    (i.e. generate help docs for all programs in my build).

9.  Figure out a way to get some default strings in for `metavar_`. Maybe just
    number them.
10. Figure out a way to decorate metavar so as to distinguish it from a literal
    value. i.e. "<metavar>".