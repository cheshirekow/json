set(_headers
    builder.h
    item.h
    json.h
    parse.h
    pipeline.h
    stream_macros.h
    type_registry.h
    util.h
    variant.h)

set(_sources
    builder.cc
    item.cc
    json.cc
    parse.cc
    pipeline.cc
    type_registry.cc
    util.cc
    variant.cc)

get_version_from_header(json.h JSON_VERSION)

cc_library(
  json STATIC
  SRCS ${_sources}
  DEPS fmt re2 util
  PKGDEPS libglog
  PROPERTIES ARCHIVE_OUTPUT_NAME tangent-json
             VERSION "${JSON_API_VERSION}"
             SOVERSION "${JSON_SO_VERSION}")

cc_library(
  json-shared SHARED
  SRCS ${_sources}
  DEPS fmt-shared re2-shared util-shared
  PKGDEPS libglog
  PROPERTIES LIBRARY_OUTPUT_NAME tangent-json
             VERSION "${JSON_API_VERSION}"
             SOVERSION "${JSON_SO_VERSION}")

set(_extra)
if("${TANGENT_BUILD_CONTEXT}" STREQUAL "DEBIAN_PACKAGE")
  set(_extra EXCLUDE_FROM_ALL)
endif()

cc_binary(
  json.exe ${_extra}
  SRCS main.cc
  DEPS argue json
  PROPERTIES OUTPUT_NAME json)

format_and_lint(
  json
  ${_headers}
  ${_sources}
  CMakeLists.txt
  doc/CMakeLists.txt
  doc/conf.py
  json_gen.py
  main.cc
  EXCLUDE "test/.*" "semver-.*\\.cmake")

configure_file(libtangent-json.pc
               ${CMAKE_CURRENT_BINARY_DIR}/libtangent-json.pc @ONLY)

install(
  TARGETS json json-shared
  ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
  LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}")
install(FILES ${_headers}
        DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/tangent/json")
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/libtangent-json.pc
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig")

add_subdirectory(test)
add_subdirectory(doc)
