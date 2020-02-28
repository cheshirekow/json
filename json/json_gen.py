#!/usr/bin/env python
"""
Generate headers/sources for json-serializable streaming interface. This
does the same thing as the JSON_DECL and JSON_DEFN macros but allows for
more readable debugging as the source code isn't hidden beneath the
preprocessor macros.
"""
from __future__ import print_function
from __future__ import unicode_literals

import argparse
import io
import logging
import os
import re
import sys

# NOTE(josh): jinja imports the native python `json` package. If we're not
# careful the build might try to use this directory as a python packge (e.g.
# bazel with legacy_create_init=True, the default).
import jinja2


def escapename(qualified_name):
  return re.sub(r"\W", "_", qualified_name)


class Context(object):
  """
  Passed in as part of the global namespace when executing a definition file.
  """

  def __init__(self):
    self.header_includes = []
    self.source_includes = []
    self.specs = []
    self.namespaces = []
    self.include_global_registration = True

  def add_header_includes(self, includes):
    """
    Add to the list of headers to include in the generated source file.
    """
    if not isinstance(includes, (list, tuple)):
      raise ValueError("add_header_includes must be a list or tuple, not {}"
                       .format(type(includes)))
    self.header_includes.extend(includes)

  def add_source_includes(self, includes):
    """
    Add to the list of headers to include in the generated source file.
    """
    if not isinstance(includes, (list, tuple)):
      raise ValueError("add_source_includes must be a list or tuple, not {}"
                       .format(type(includes)))
    self.source_includes.extend(includes)

  def decl_json(self, decl, fields=None, namespace=None):
    """
    Declare one or more structures to be json serializable, and generate
    overloads for those types. decl may either be the name of a structure,
    or a dictionary mapping names of structure to lists of fields.
    """
    if isinstance(decl, dict):
      for struct, dfields in decl.items():
        self.decl_json(struct, dfields, namespace)
    else:
      if not isinstance(decl, str):
        raise ValueError("decl must be a string, not {}".format(type(fields)))
      if not isinstance(fields, (list, tuple)):
        raise ValueError("fields must be a list or tuple, not {}"
                         .format(type(fields)))

      if namespace is not None:
        fqn = '{}::{}'.format(namespace, decl)
      else:
        fqn = decl

      self.specs.append((fqn, fields))


def process_file(infile_content):
  ctx = Context()
  _globals = {
      "add_header_includes": ctx.add_header_includes,
      "add_source_includes": ctx.add_source_includes,
      "decl_json": ctx.decl_json
  }
  exec(infile_content, _globals)  # pylint: disable=W0122
  return ctx


def main():
  logging.basicConfig(level=logging.INFO)

  parser = argparse.ArgumentParser(description=__doc__)
  parser.add_argument("-d", "--debug", action="store_true",
                      help=argparse.SUPPRESS)
  parser.add_argument("-o", "--outdir", default=os.getcwd(),
                      help="directory where to put output files. Default is "
                           "cwd")
  parser.add_argument("-b", "--basename",
                      help="basename of output files. Default is basename of "
                           "input file.")
  parser.add_argument("infiles", nargs="+",
                      help="specification files to process")
  args = parser.parse_args()

  if args.basename is not None:
    assert len(args.infile) == 1, (
        "-b/--basename option is only allowed if a single infile is given")

  thisdir = os.path.dirname(os.path.realpath(__file__))
  env = jinja2.Environment(
      loader=jinja2.FileSystemLoader([
          os.getcwd(),
          thisdir,
      ])
  )
  env.globals.update(__builtins__.__dict__)
  env.globals.update({"escapename": escapename})
  header_template = env.get_template("json_gen.h.tpl")
  source_template = env.get_template("json_gen.cc.tpl")

  for infilepath in args.infiles:
    basename = os.path.basename(infilepath)
    basename = os.path.splitext(basename)[0]
    if args.basename is not None:
      basename = args.basename

    with io.open(infilepath, "r", encoding="utf-8") as infile:
      infile_content = infile.read()

    ctx = process_file(infile_content)

    outfilebase = os.path.join(args.outdir, basename)
    headerfile_path = outfilebase + '.h'
    sourcefile_path = outfilebase + '.cc'

    header_content = header_template.render(ctx=ctx)
    source_content = source_template.render(
        ctx=ctx, headerpath=(basename + ".h"))

    if args.debug:
      sys.stdout.write(headerfile_path)
      sys.stdout.write("\n")
      sys.stdout.write("=" * len(headerfile_path))
      sys.stdout.write("\n\n")
      sys.stdout.flush()
      sys.stdout.write(header_content)
      sys.stdout.write("\n")

      sys.stdout.write(sourcefile_path)
      sys.stdout.write("\n")
      sys.stdout.write("=" * len(sourcefile_path))
      sys.stdout.write("\n\n")
      sys.stdout.flush()
      sys.stdout.write(source_content)
      sys.stdout.write("\n")
    else:
      with io.open(headerfile_path, "w", encoding="utf-8") as outfile:
        outfile.write(header_content)
        outfile.write("\n")

      with io.open(sourcefile_path, "w", encoding="utf-8") as outfile:
        outfile.write(source_content)
        outfile.write("\n")


if __name__ == "__main__":
  main()
