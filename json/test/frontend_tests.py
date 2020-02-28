# -*- coding: utf-8 -*-
# pylint: disable=R1708
from __future__ import unicode_literals

import argparse
import contextlib
import difflib
import functools
import inspect
import io
import logging
import os
import re
import six
import subprocess
import sys
import tempfile
import unittest

logger = logging.getLogger(__name__)

GLOBALS = {
  "exe_path": None
}

# NOTE(josh): backport from functools.py in python 3.6 so that we can use it in
# python 2.7
if sys.version_info < (3, 5, 0):
  # pylint: disable=all
  class partialmethod(object):
    """Method descriptor with partial application of the given arguments
    and keywords.

    Supports wrapping existing descriptors and handles non-descriptor
    callables as instance methods.
    """

    def __init__(self, func, *args, **keywords):
      if not callable(func) and not hasattr(func, "__get__"):
        raise TypeError("{!r} is not callable or a descriptor"
                        .format(func))

      # func could be a descriptor like classmethod which isn't callable,
      # so we can't inherit from partial (it verifies func is callable)
      if isinstance(func, partialmethod):
        # flattening is mandatory in order to place cls/self before all
        # other arguments
        # it's also more efficient since only one function will be called
        self.func = func.func
        self.args = func.args + args
        self.keywords = func.keywords.copy()
        self.keywords.update(keywords)
      else:
        self.func = func
        self.args = args
        self.keywords = keywords

    def __repr__(self):
      args = ", ".join(map(repr, self.args))
      keywords = ", ".join("{}={!r}".format(k, v)
                           for k, v in self.keywords.items())
      format_string = "{module}.{cls}({func}, {args}, {keywords})"
      return format_string.format(module=self.__class__.__module__,
                                  cls=self.__class__.__qualname__,
                                  func=self.func,
                                  args=args,
                                  keywords=keywords)

    def _make_unbound_method(self):
      def _method(*args, **keywords):
        call_keywords = self.keywords.copy()
        call_keywords.update(keywords)
        cls_or_self = args[0]
        rest = args[:]
        call_args = (cls_or_self,) + self.args + tuple(rest)
        return self.func(*call_args, **call_keywords)
      _method.__isabstractmethod__ = self.__isabstractmethod__
      _method._partialmethod = self
      return _method

    def __get__(self, obj, cls):
      get = getattr(self.func, "__get__", None)
      result = None
      if get is not None:
        new_func = get(obj, cls)
        if new_func is not self.func:
          # Assume __get__ returning something new indicates the
          # creation of an appropriate callable
          result = functools.partial(new_func, *self.args, **self.keywords)
          try:
            result.__self__ = new_func.__self__
          except AttributeError:
            pass
      if result is None:
        # If the underlying descriptor didn't do anything, treat this
        # like an instance method
        result = self._make_unbound_method().__get__(obj, cls)
      return result

    @property
    def __isabstractmethod__(self):
      return getattr(self.func, "__isabstractmethod__", False)
else:
  from functools import partialmethod


def assert_textequal(test, input_str, actual_str, expect_str):
  """
  Assert that the two strings are equivalent. Format a useful error message with
  unified diff if they are not.
  """

  if sys.version_info[0] < 3:
    assert isinstance(input_str, unicode)

  delta_lines = list(difflib.unified_diff(expect_str.split('\n'),
                                          actual_str.split('\n')))
  delta = '\n'.join(delta_lines[2:])

  if actual_str != expect_str:
    message = ('Input text:\n-----------------\n{}\n'
               'Output text:\n-----------------\n{}\n'
               'Expected Output:\n-----------------\n{}\n'
               'Diff:\n-----------------\n{}'
               .format(input_str,
                       actual_str,
                       expect_str,
                       delta))
    if sys.version_info[0] < 3:
      message = message.encode('utf-8')
    raise AssertionError(message)


def exec_sidecar(
    test, test_json, expect_lex=None, expect_parse=None,
    expect_markup=None):
  """
  Assert a formatting and, optionally, a lex, parse, or layout tree.
  """

  exe_path = GLOBALS["exe_path"]
  with tempfile.NamedTemporaryFile(delete=False) as testfile:
    testfile.write(test_json.encode("utf-8"))
    testfile_path = testfile.name

  if expect_lex is not None:
    actual_lex = subprocess.check_output(
        [exe_path, "lex", testfile_path]).decode("utf-8")
    assert_textequal(test, test_json, actual_lex, expect_lex)
  if expect_parse is not None:
    actual_parse = subprocess.check_output(
        [exe_path, "parse", testfile_path]).decode("utf-8")
    assert_textequal(test, test_json, actual_parse, expect_parse)
  if expect_markup is not None:
    actual_markup = subprocess.check_output(
        [exe_path, "markup", "--omit-template",
         testfile_path]).decode("utf-8")
    assert_textequal(test, test_json, expect_markup, actual_markup)


class SidecarMeta(type):
  """
  Since the unittest framework inspects class members prior to calling
  ``setUpClass`` there does not appear to be any additional hooks that we
  can use to automatically load sidecars. We use a metaclass so that when the
  test fixture class object is instanciated (class is defined) we can load the
  sidecars. This way test methods are loaded before ``unittest`` inspects the
  class.
  """
  def __new__(cls, name, bases, dct):
    subcls = type.__new__(cls, name, bases, dct)
    if name not in ("MetaBase", "TestBase"):
      subcls.load_sidecar()
    return subcls


def to_camelcase(query):
  """
  Convert a title string into lower case
  """
  return re.sub(r"\s+", "_", query.lower().strip())


def consume_codefence(lineiter):
  """
  Consume document text between codefences
  """
  for lineno, line in lineiter:
    if line.startswith("```"):  # first fence
      break

  linebuf = []
  for lineno, line in lineiter:
    if line.startswith("```"):  # second fence
      return "".join(linebuf)
    linebuf.append(line)


def consume_html(lineiter):
  """
  Consume document text between codefences
  """
  linebuf = []

  # consume any padding
  for lineno, line in lineiter:
    if line.startswith("<div"):
      break

  for lineno, line in lineiter:
    if line.startswith("</div"):
      return "".join(linebuf)
    linebuf.append(line)


def consume_one_test(lineiter):
  """
  Consume one test specification from a markdown sidecar file.
  """
  next_test = None
  test_case = None
  expect_lex = None
  expect_parse = None
  expect_markup = None

  line = None
  for lineno, line in lineiter:
    if line.startswith("## "):
      next_test = to_camelcase(line.lstrip("#").lstrip())
      return test_case, expect_lex, expect_parse, expect_markup, next_test
    if line.startswith("### Test Case:"):
      test_case = consume_codefence(lineiter)
    elif line.startswith("### Expect Lex:"):
      expect_lex = consume_codefence(lineiter)
    elif line.startswith("### Expect Parse:"):
      expect_parse = consume_codefence(lineiter)
    elif line.startswith("### Expect Markup:"):
      expect_markup = consume_html(lineiter)

  return test_case, expect_lex, expect_parse, expect_markup, next_test


class FrontendTests(six.with_metaclass(SidecarMeta, unittest.TestCase)):
  """
  Given a bunch of example json strings, ensure that they
  lex, parse, markup the same as expected.
  """
  kNumSidecarTests = 0
  kExpectNumSidecarTests = 2

  @classmethod
  def setUpClass(self):
    thisdir = os.path.dirname(os.path.realpath(__file__))

  @classmethod
  def append_sidecar_test(
    cls, test_name, input_str, expect_lex, expect_parse, expect_markup):
    """
    Add a new test loaded from the cmake sidecar file
    """

    closure = partialmethod(
      exec_sidecar, input_str, expect_lex, expect_parse,
      expect_markup)
    # TODO(josh): figure out how to set the docstring correctly, this doesn't
    # seem to work
    closure.__doc__ = ""
    setattr(cls, "test_" + test_name, closure)
    cls.kNumSidecarTests += 1

  @classmethod
  def load_sidecar(cls, filepath=None):
    if filepath is None:
      filepath = inspect.getfile(cls)
    markdown_sidecar = filepath[:-3] + ".md"
    if not os.path.exists(markdown_sidecar):
      return
    with io.open(markdown_sidecar, "r", encoding="utf-8") as infile:
      lineiter = enumerate(infile)
      _, _, _, _, next_test = consume_one_test(lineiter)
      while next_test is not None:
        test_name = next_test
        (test_case, expect_lex, expect_parse, expect_markup,
         next_test) = consume_one_test(lineiter)
        assert test_case is not None, (
          "Error, no test_case for {}".format(test_name)
        )
        cls.append_sidecar_test(
          test_name, test_case, expect_lex, expect_parse, expect_markup)

  def test_numsidecar(self):
    """
    Sanity check to makesure all sidecar tests are run.
    """
    self.assertEqual(self.kExpectNumSidecarTests, self.kNumSidecarTests)

  def __init__(self, *args, **kwargs):
    super(FrontendTests, self).__init__(*args, **kwargs)

  @contextlib.contextmanager
  def subTest(self, msg=None, **params):
    # pylint: disable=no-member
    if sys.version_info < (3, 4, 0):
      yield None
    else:
      yield super(TestBase, self).subTest(msg=msg, **params)


def main():
  parser = argparse.ArgumentParser(description=__doc__)
  parser.add_argument("--exe-path", required=True)
  parser.add_argument("remainder", nargs=argparse.REMAINDER)
  args = parser.parse_args()

  GLOBALS["exe_path"] = args.exe_path
  sys.argv[1:] = args.remainder
  unittest.main()


if __name__ == "__main__":
  main()
