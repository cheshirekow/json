if("${TANGENT_BUILD_CONTEXT}" STREQUAL "DEBIAN_PACKAGE")
  # TODO(josh): move this to some kind of debian/stubs/json.cmake kind of
  # location
  cc_library(
    json STATIC IMPORTED
    PROPERTIES IMPORTED_LOCATION /usr/${CMAKE_INSTALL_LIBDIR}/libtangent-json.a
               INTERFACE_INCLUDE_DIRECTORIES
               /usr/${CMAKE_INSTALL_INCLUDEDIR}/tangent)
  set_property(TARGET json PROPERTY IMPORTED_LINK_INTERFACE_LIBRARIES fmt re2)

  cc_library(
    json-shared SHARED IMPORTED
    PROPERTIES IMPORTED_LOCATION
               /usr/${CMAKE_INSTALL_LIBDIR}/libtangent-json.so
               INTERFACE_INCLUDE_DIRECTORIES
               /usr/${CMAKE_INSTALL_INCLUDEDIR}/tangent)
  set_property(TARGET json-shared PROPERTY IMPORTED_LINK_INTERFACE_LIBRARIES
                                           fmt-shared re2-shared)
endif()

set(_headers
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
set(_sources action.cc exception.cc kwargs.cc parse.cc parser.cc glog.cc)

get_version_from_header(argue.h ARGUE_VERSION)

cc_library(
  argue STATIC
  SRCS ${_sources}
  DEPS fmt json util
  PKGDEPS libglog)

cc_library(
  argue-shared SHARED
  SRCS ${_sources}
  DEPS fmt-shared json-shared util-shared
  PKGDEPS libglog
  PROPERTIES LIBRARY_OUTPUT_NAME argue
             VERSION "${ARGUE_API_VERSION}"
             SOVERSION "${ARGUE_SO_VERSION}")

cc_binary(
  argue-can-complete
  SRCS can_complete.cc
  DEPS argue loki
  PKGDEPS libelf)

format_and_lint(
  argue
  ${_headers}
  ${_sources}
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
  EXCLUDE "semver-.*\\.cmake" doc/stage/conf.py)

configure_file(libargue.pc ${CMAKE_CURRENT_BINARY_DIR}/libargue.pc @ONLY)

install(
  TARGETS argue argue-shared argue-can-complete
  RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
  ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
  LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}")
install(FILES ${_headers} DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/argue")
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/libargue.pc
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig")

add_subdirectory(doc)
add_subdirectory(examples)
add_subdirectory(test)
