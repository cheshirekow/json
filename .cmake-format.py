with section("parse"):
  additional_commands = {
    "cc_binary": {
      "pargs": 1,
      "kwargs": {
          "SRCS": "*",
          "DEPS": "*",
          "PKGDEPS": "*"
      }
    },
    "cc_library": {
      "pargs": "1+",
      "flags": ["STATIC", "SHARED"],
      "kwargs": {
          "SRCS": "*",
          "DEPS": "*",
          "PKGDEPS": "*"
      }
    },
    "cc_test": {
      "pargs": 1,
      "kwargs": {
          "SRCS": "*",
          "DEPS": "*",
          "PKGDEPS": "*",
          "ARGV": "*",
          "TEST_DEPS": "*",
          "WORKING_DIRECTORY": "*",
          "LABELS": "*"
      }
    },
    "pkg_find": {
      "kwargs": {
        "PKG": "*"
      }
    },
    "format_and_lint": {
      "kwargs": {
        "CC": "*",
        "CCDEPENDS": "*",
        "CMAKE": "*",
        "PY": "*",
        "JS": "*",
        "EXCLUDE": "*",
        "SHELL": "*"
      }
    }
  }
