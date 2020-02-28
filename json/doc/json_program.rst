================
The json program
================

Included in the package is a simple json utility application intended to
demonstrate usage of the library. The command `json` can dump the lex'ed token
stream or the parsed event stream. It can also validate a json file or markup
its contents with html that can be used to publish semantic-highlighted
json documents.

::

    ====
    json
    ====
    version: 0.2.0
    author : Josh Bialkowski <josh.bialkowski@gmail.com>
    copyright: (C) 2018

    json [-h/--help] [-v/--version] <command>

    Demonstrates the usage of the json library to lex and parse JSON data

    Flags:
    ------
    -h  --help          print this help message
    -v  --version       print version information and exit

    Positionals:
    ------------
    command             Each subcommand has it's own options and arguments, see
                        individual subcommand help.

    Subcommands:
    ------------
    lex                 Lex the file and dump token information
    markup              Parse and dump the contents with HTML markup
    parse               Parse the file and dump actionable parse events
    verify              Parse the file and exit with 0 if it's valid json
