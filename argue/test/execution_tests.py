#!/usr/bin/env python
"""
Execute the argue demo program and verify that the output is as expected for
varius inputs.
"""

from __future__ import print_function
from __future__ import unicode_literals

import argparse
import difflib
import os
import subprocess
import sys
import unittest

# NOTE(josh): eww globals, but unittest doesn't really make this easy
EXE_PATH = None


EXPECT_HELP = """\
==========
argue-demo
==========
version: 0.0.1
author : Josh Bialkowski <josh.bialkowski@gmail.com>
copyright: (C) 2018

argue-demo [-h/--help] [-v/--version] [-s/--sum] <N> [N..]

Flags:
------
-h  --help          print this help message
-v  --version       print version information and exit
-s  --sum           sum the integers (default: find the max)

Positionals:
------------
integer             an integer for the accumulator
"""


class TestExecution(unittest.TestCase):
  def __init__(self, method_name):
    super(TestExecution, self).__init__(method_name)

  def assertNoDiff(self, expected_text, actual_text):
    diffgen = difflib.unified_diff(expected_text.split("\n"),
                                   actual_text.split("\n"))
    delta = '\n'.join(diffgen)
    message = ("Expected equal text:\n"
               "Actual text: \n"
               "```\n{}\n```\n\n"
               "Expected text: \n"
               "```\n{}\n```\n\n"
               "Delta:\n"
               "```\n{}\n```\n".format(expected_text, actual_text, delta))
    self.assertEqual(expected_text, actual_text, msg=message)

  def call_program(self, arguments, expect_stdout=None, expect_stderr=None):
    """
    Call the program with the given arguments list. Return a tuple of
    (retcode, stdout, stderr)
    """
    if expect_stderr is None:
      expect_stderr = ""
    if expect_stdout is None:
      expect_stdout = ""

    proc = subprocess.Popen(["argue-demo"] + arguments, executable=EXE_PATH,
                            stderr=subprocess.PIPE, stdout=subprocess.PIPE)
    stdout, stderr = proc.communicate()
    stdout = stdout.decode("utf-8")
    stderr = stderr.decode("utf-8")
    self.assertEqual(proc.returncode, 0)
    self.assertNoDiff(expect_stdout, stdout)
    self.assertNoDiff(expect_stderr, stderr)

  def test_help(self):
    self.call_program(["--help"], expect_stderr=EXPECT_HELP)

  def test_max_example(self):
    self.call_program(["1", "2", "3", "4"], "max(1, 2, 3, 4) = 4\n")

  def test_sum_example(self):
    self.call_program(["--sum", "1", "2", "3", "4"],
                      "sum(1, 2, 3, 4) = 10\n")


def suite():
  return unittest.TestLoader().loadTestsFromTestCase(TestExecution)


class HelpAction(argparse._HelpAction):  # pylint:disable=W0212
  def __call__(self, parser, namespace, values, option_string=None):
    parser.print_help()
    unittest.main(argv=[sys.argv[0], "--help"])


EPILOG = """
This program wraps the unittest.main() program so see additional arguments
below.

unittest.main()
===============
"""


def main():
  parser = argparse.ArgumentParser(
      add_help=False,
      description=__doc__,
      epilog=EPILOG,
      formatter_class=argparse.RawTextHelpFormatter)
  parser.add_argument("-h", "--help", action=HelpAction)
  parser.add_argument("--exe-path", required=True,
                      help="path to the argue-demo exe file")
  parser.add_argument("remainder", nargs=argparse.REMAINDER)
  args = parser.parse_args()
  global EXE_PATH  # pylint: disable=W0603
  EXE_PATH = args.exe_path

  print("Using exe-path: {}".format(EXE_PATH))
  assert os.path.exists(EXE_PATH), \
    ("Exe path: {} does not exist with respect to "
     "{}").format(EXE_PATH, os.getcwd())
  unittest.main(argv=[sys.argv[0]] + args.remainder)


if __name__ == "__main__":
  main()
