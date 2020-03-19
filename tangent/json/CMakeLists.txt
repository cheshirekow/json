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

get_version_from_header(json.h TANGENT_JSON_VERSION)

cc_library(
  json STATIC
  SRCS ${_sources}
  DEPS fmt::fmt re2 tangent::util
  PKGDEPS libglog
  PROPERTIES ARCHIVE_OUTPUT_NAME tangent-json
             INTERFACE_INCLUDE_DIRECTORIES "$<INSTALL_INTERFACE:include>")
add_library(tangent::json ALIAS json)

cc_library(
  json-shared SHARED
  SRCS ${_sources}
  DEPS fmt::fmt re2-shared tangent::util-shared
  PKGDEPS libglog
  PROPERTIES LIBRARY_OUTPUT_NAME tangent-json
             VERSION "${TANGENT_JSON_API_VERSION}"
             SOVERSION "${TANGENT_JSON_SO_VERSION}"
             INTERFACE_INCLUDE_DIRECTORIES "$<INSTALL_INTERFACE:include>")
add_library(tangent::json-shared ALIAS json-shared)

# NOTE(josh): the json binary depends on libargue, and libargue depends on
# libtangent-json. We can't build the json binary with the libtangent-json
# debian package because it would induce a circular dependency.
if(NOT "${TANGENT_BUILD_CONTEXT}" STREQUAL DEBIAN_PACKAGE)
  cc_binary(
    json.exe
    SRCS main.cc
    DEPS argue::static tangent::json
    PROPERTIES OUTPUT_NAME json)
endif()

configure_file(libtangent-json.pc
               ${CMAKE_CURRENT_BINARY_DIR}/libtangent-json.pc @ONLY)

write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/libtangent-json-config-version.cmake"
  VERSION "${TANGENT_JSON_VERSION}"
  COMPATIBILITY AnyNewerVersion)

set(_package_location ${CMAKE_INSTALL_LIBDIR}/cmake/libtangent-json)

configure_package_config_file(
  libtangent-json-config.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/libtangent-json-config.cmake PATH_VARS
  CMAKE_INSTALL_BINDIR INSTALL_DESTINATION ${_package_location})

install(
  TARGETS json json-shared
  EXPORT libtangent-json-targets
  ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
  LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}")

install(FILES ${_headers}
        DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/tangent/json")

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/libtangent-json.pc
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig")

export(
  EXPORT libtangent-json-targets
  FILE "${CMAKE_CURRENT_BINARY_DIR}/libtangent-json-targets.cmake"
  NAMESPACE "tangent::")

install(
  EXPORT libtangent-json-targets
  FILE libtangent-json-targets.cmake
  NAMESPACE "tangent::"
  DESTINATION ${_package_location})

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/libtangent-json-config.cmake"
              "${CMAKE_CURRENT_BINARY_DIR}/libtangent-json-config-version.cmake"
        DESTINATION ${_package_location})

set(_deploy_key)
if(IS_TRAVIS_CI)
  set(_deploy_key ${CMAKE_CURRENT_BINARY_DIR}/libtangent-json.deploy_key)
  add_custom_command(
    OUTPUT ${_deploy_key}
    COMMAND
      # cmake-format: off
      openssl aes-256-cbc
      -K $ENV{encrypted_3bbe8f6101ca_key}
      -iv $ENV{encrypted_3bbe8f6101ca_iv}
      -in ${CMAKE_SOURCE_DIR}/tangent/tooling/deploy_keys/libtangent-json.enc
      -out ${_deploy_key}
      -d
      # cmake-format: on
    COMMAND chmod 0600 ${_deploy_key}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Decrypting libtangent-json.deploy_key")
endif()

add_custom_target(
  push-json-github-pseudorelease-tag
  COMMAND python -Bm tangent.tooling.github create-pseudorelease-tag
          cheshirekow/json \$\${TRAVIS_BRANCH}
  DEPENDS ${_deploy_key}
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  COMMENT "Pushing doc artifacts to the RTD repository")

add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/release_notes-${TANGENT_JSON_VERSION}.rst
  COMMAND
    python -Bm tangent.tooling.get_release_notes json/doc/release_notes.rst
    \$\${TRAVIS_TAG} -o
    ${CMAKE_CURRENT_BINARY_DIR}/release_notes-${TANGENT_JSON_VERSION}.rst
  DEPENDS doc/release_notes.rst
          ${CMAKE_SOURCE_DIR}/tangent/tooling/get_release_notes.py
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/release_notes-${TANGENT_JSON_VERSION}.md
  COMMAND
    pandoc -s -o
    ${CMAKE_CURRENT_BINARY_DIR}/release_notes-${TANGENT_JSON_VERSION}.md
    ${CMAKE_CURRENT_BINARY_DIR}/release_notes-${TANGENT_JSON_VERSION}.rst
  DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/release_notes-${TANGENT_JSON_VERSION}.rst
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

add_subdirectory(test)
add_subdirectory(doc)

add_custom_target(
  push-json-github-release
  COMMAND
    # cmake-format: off
    python -Bm tangent.tooling.github push-release
      --message ${CMAKE_CURRENT_BINARY_DIR}/release_notes-${TANGENT_JSON_VERSION}.md
      cheshirekow/json \$\${TRAVIS_TAG}
      # ${CMAKE_BINARY_DIR}/debian/libtangent-json_${TANGENT_JSON_VERSION}.orig.tar.gz
    # cmake-format: on
  DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/release_notes-${TANGENT_JSON_VERSION}.md
  COMMENT "Uploading json release artifacts"
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
