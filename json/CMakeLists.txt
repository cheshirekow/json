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
  EXCLUDE "doc/stage" "test/.*" "semver-.*\\.cmake")

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

if(IS_TRAVIS_CI)
  add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/libtangent-json.deploy_key
    COMMAND
      # cmake-format: off
      openssl aes-256-cbc
      -K $ENV{encrypted_3bbe8f6101ca_key}
      -iv $ENV{encrypted_3bbe8f6101ca_iv}
      -in ${CMAKE_SOURCE_DIR}/tangent/tooling/deploy_keys/libtangent-json.enc
      -out ${CMAKE_CURRENT_BINARY_DIR}/libtangent-json.deploy_key
      -d
      # cmake-format: on
    COMMAND chmod 0600 ${CMAKE_CURRENT_BINARY_DIR}/libtangent-json.deploy_key
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Decrypting libtangent-json.deploy_key")
endif()

add_custom_target(
  push-json-github-pseudorelease-tag
  COMMAND python -Bm tangent.tooling.github create-pseudorelease-tag
          cheshirekow/json \$\${TRAVIS_BRANCH}
  DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/libtangent-json.deploy_key
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  COMMENT "Pushing doc artifacts to the RTD repository")

add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/release_notes-${JSON_VERSION}.rst
  COMMAND
    python -Bm tangent.tooling.get_release_notes json/doc/release_notes.rst
    \$\${TRAVIS_TAG} -o
    ${CMAKE_CURRENT_BINARY_DIR}/release_notes-${JSON_VERSION}.rst
  DEPENDS doc/release_notes.rst
          ${CMAKE_SOURCE_DIR}/tangent/tooling/get_release_notes.py
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/release_notes-${JSON_VERSION}.md
  COMMAND
    pandoc -s -o ${CMAKE_CURRENT_BINARY_DIR}/release_notes-${JSON_VERSION}.md
    ${CMAKE_CURRENT_BINARY_DIR}/release_notes-${JSON_VERSION}.rst
  DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/release_notes-${JSON_VERSION}.rst
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

add_subdirectory(test)
add_subdirectory(doc)

add_custom_target(
  push-json-github-release
  COMMAND
    # cmake-format: off
    python -Bm tangent.tooling.github push-release
      --message ${CMAKE_CURRENT_BINARY_DIR}/release_notes-${JSON_VERSION}.md
      cheshirekow/json \$\${TRAVIS_TAG}
      # ${CMAKE_BINARY_DIR}/debian/libtangent-json_${JSON_VERSION}.orig.tar.gz
    # cmake-format: on
  DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/release_notes-${JSON_VERSION}.md
  COMMENT "Uploading json release artifacts"
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
