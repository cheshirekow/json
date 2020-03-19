import os

with open(os.path.join("./conf_common.py")) as infile:
  exec(infile.read())  # pylint: disable=W0122

project = "json"
docname = project + u'doc'
title = project + ' Documentation'
version = "@JSON_VERSION@"

assert version is not None
release = version
