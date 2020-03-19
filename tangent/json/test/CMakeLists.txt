add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/test_types.h
         ${CMAKE_CURRENT_BINARY_DIR}/test_types.cc
  COMMAND python ${CMAKE_CURRENT_SOURCE_DIR}/../json_gen.py --outdir
          ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/test_types.py
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/test_types.py
          ${CMAKE_CURRENT_SOURCE_DIR}/../json_gen.py #
          ${CMAKE_CURRENT_SOURCE_DIR}/../json_gen.h.tpl
          ${CMAKE_CURRENT_SOURCE_DIR}/../json_gen.cc.tpl
          ${CMAKE_CURRENT_LISTFILE})

add_custom_target(
  genfiles-json DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/test_types.h
                        ${CMAKE_CURRENT_BINARY_DIR}/test_types.cc)
add_dependencies(lint-main genfiles-json)

cc_test(
  json-lexer_test
  SRCS lexer_test.cc
  DEPS gtest gtest_main json
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

cc_test(
  json-parser_test
  SRCS parser_test.cc
  DEPS gtest gtest_main json
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

cc_test(
  json-builder_test
  SRCS builder_test.cc
  DEPS gtest gtest_main json
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

cc_test(
  json-variant_test
  SRCS variant_test.cc
  DEPS gtest gtest_main json
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

cc_test(
  json-stream_test
  SRCS stream_test.cc
  DEPS gtest gtest_main json
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

cc_test(
  json-macro_test
  SRCS macro_test.cc
  DEPS gtest gtest_main json
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

# NOTE(josh): debian package build environment does not include jinja2 so we
# can't run the codegen and we can't build the tests
if("${TANGENT_BUILD_CONTEXT}" STREQUAL DEBIAN_PACKAGE)
  return()
endif()

cc_test(
  json-stream_gen_test
  SRCS ${CMAKE_CURRENT_BINARY_DIR}/test_types.cc stream_gen_test.cc
  DEPS gtest gtest_main json
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
target_include_directories(json-stream_gen_test PRIVATE ${CMAKE_BINARY_DIR})

cc_test(
  json-type_registry_test
  SRCS type_registry_test.cc
  DEPS gtest gtest_main json
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
target_include_directories(json-type_registry_test PRIVATE ${CMAKE_BINARY_DIR})

add_test(
  NAME json-frontend-tests
  COMMAND python frontend_tests.py --exe-path $<TARGET_FILE:json.exe>
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
