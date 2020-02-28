set(argue_headers
    argue.h
    action.h
    action.tcc
    exception.h
    glog.h
    keywords.h
    keywords.tcc
    kwargs.h
    kwargs.tcc
    parse.h
    parse.tcc
    parser.h
    parser.tcc
    storage_model.h
    storage_model.tcc
    util.h)
set(argue_sources action.cc exception.cc kwargs.cc parse.cc parser.cc glog.cc)

cc_library(
  argue STATIC
  SRCS ${argue_sources}
  DEPS fmt json
  PKGDEPS libglog)

cc_binary(
  argue-can-complete
  SRCS can_complete.cc
  DEPS argue loki
  PKGDEPS libelf)

format_and_lint(
  argue
  ${argue_headers}
  ${argue_sources}
  BUILD
  can_complete.cc
  examples/BUILD
  examples/CMakeLists.txt
  examples/argparse_example.cc
  examples/simple_example.cc
  examples/subparser_example.cc
  test/arg_test.cc
  test/assert_test.cc
  test/BUILD
  test/CMakeLists.txt
  test/keyword_test.cc
  test/parse_test.cc
  test/util_test.cc
  CMakeLists.txt
  doc/CMakeLists.txt
  doc/conf.py
  test/execution_tests.py
  SHELL bash_completion.d/argue-argcomplete
  EXCLUDE doc/argue-semver.cmake doc/stage/conf.py)

add_subdirectory(doc)
add_subdirectory(examples)
add_subdirectory(test)
