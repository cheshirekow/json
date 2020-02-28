"""
Get the version string by parsing a C++ header file. The version is expected
to be in the form of:

#define FOO_VERSION \
  { 0, 1, 0, "dev", 0 }
"""

import argparse
import os
import io
import sys

OUTFILE_TPL = """
set({macro} "{version}")
"""


def main():
  argparser = argparse.ArgumentParser(description=__doc__)
  argparser.add_argument("macro")
  argparser.add_argument("filepath")
  argparser.add_argument("outfilepath", nargs="?", default="-")
  args = argparser.parse_args()
  if args.outfilepath == "-":
    args.outfilepath = os.dup(sys.stdout.fileno())

  version = None
  with io.open(args.filepath, "r", encoding="utf-8") as infile:
    lineiter = iter(infile)
    for line in lineiter:
      if line.startswith("#define " + args.macro):
        version = next(lineiter)
        break

  if version is None:
    return 1
  # Strip whitespace and brackets:
  version = version.strip()[1:-1]
  # Split along commas
  verparts = [part.strip().strip('"') for part in version.split(",")]

  semver = ".".join(verparts[:3])
  pre_release = "".join(verparts[3:])
  if pre_release:
    semver += "-" + pre_release

  with io.open(args.outfilepath, "w", encoding="utf-8") as outfile:
    outfile.write("set({} \"{}\")\n".format(args.macro, semver))
  return 0


if __name__ == "__main__":
  sys.exit(main())
