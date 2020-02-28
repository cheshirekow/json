import os

this_file = os.path.realpath(__file__)
this_dir = os.path.dirname(this_file)
proj_dir = os.path.dirname(this_dir)
root_dir = os.path.dirname(proj_dir)

with open(os.path.join(root_dir, "doc/conf.py")) as infile:
  exec(infile.read())  # pylint: disable=W0122

project = "json"
docname = project + u'doc'
title = project + ' Documentation'
version = None

with open(os.path.join(proj_dir, "json.h")) as infile:
  lines = iter(infile)
  for line in lines:
    if line.startswith("#define JSON_VERSION \\"):
      version = ".".join(next(lines).strip().strip("{}").split(", "))

assert version is not None
release = version

html_static_path = [os.path.join(root_dir, "doc/sphinx-static")]
