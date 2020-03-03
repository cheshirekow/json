with section("parse"):
  additional_commands = {
    "cc_binary": {
      "pargs": "1+",
      "kwargs": {
          "SRCS": "*",
          "DEPS": "*",
          "PKGDEPS": "*",
          "PROPERTIES": {
            "kwargs": {
              "OUTPUT_NAME": 1,
            }
          }
      }
    },
    "cc_library": {
      "pargs": "1+",
      "flags": ["STATIC", "SHARED"],
      "kwargs": {
          "SRCS": "*",
          "DEPS": "*",
          "PKGDEPS": "*",
          "PROPERTIES": {
            "kwargs": {
              "LIBRARY_OUTPUT_NAME": 1,
              "VERSION": 1,
              "SOVERSION": 1
            }
          }
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
    "check_call": {
      "kwargs": {
        "COMMAND": "*",
        "WORKING_DIRECTORY": "1",
        "TIMEOUT": "1",
        "RESULT_VARIABLE": "1",
        "RESULTS_VARIABLE": "1",
        "OUTPUT_VARIABLE": "1",
        "ERROR_VARIABLE": "1",
        "INPUT_FILE": "1",
        "OUTPUT_FILE": "1",
        "ERROR_FILE": "1",
        "ENCODING": "1",
      },
      "flags": [
        "OUTPUT_QUIET",
        "ERROR_QUIET",
        "OUTPUT_STRIP_TRAILING_WHITESPACE",
        "ERROR_STRIP_TRAILING_WHITESPACE",
      ]
    },
    "create_debian_binary_packages": {
      "pargs": [3, "+"],
      "kwargs": {
        "OUTPUTS": "*",
        "DEPS": "*"
      }
    },
    "create_debian_packages": {
      "pargs": [
        {"nargs": "+", "flags": ["FORCE_PBUILDER"]},
      ],
      "kwargs": {
        "OUTPUTS": "*",
        "DEPS": "*"
      }
    },
    "pkg_find": {
      "kwargs": {
        "PKG": "*"
      }
    },
    "exportvars": {
      "pargs": "1+",
      "kwargs": {
        "VARS": "+"
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
    },
    "importvars": {
      "pargs": "1+",
      "kwargs": {
        "VARS": "+"
      }
    },
    "stage_files": {
      "kwargs": {
        "LIST": 1,
        "STAGE": 1,
        "SOURCEDIR": 1,
        "FILES": "*"
      }
    }
  }
