add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/test_types.h
         ${CMAKE_CURRENT_BINARY_DIR}/test_types.cc
  COMMAND python3 ${CMAKE_SOURCE_DIR}/json/json_gen.py --outdir
          ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/test_types.py
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/test_types.py
          ${CMAKE_SOURCE_DIR}/json/json_gen.py #
          ${CMAKE_SOURCE_DIR}/json/json_gen.h.tpl
          ${CMAKE_SOURCE_DIR}/json/json_gen.cc.tpl
          ${CMAKE_CURRENT_LISTFILE})

add_executable(json-lexer_test lexer_test.cc)
target_link_libraries(json-lexer_test gtest gtest_main json)
add_test(
  NAME json-lexer_test
  COMMAND json-lexer_test
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

add_executable(json-parser_test parser_test.cc)
target_link_libraries(json-parser_test gtest gtest_main json)
add_test(
  NAME json-parser_test
  COMMAND json-parser_test
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

add_executable(json-builder_test builder_test.cc)
target_link_libraries(json-builder_test gtest gtest_main json)
add_test(
  NAME json-builder_test
  COMMAND json-builder_test
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

add_executable(json-variant_test variant_test.cc)
target_link_libraries(json-variant_test gtest gtest_main json)
add_test(
  NAME json-variant_test
  COMMAND json-variant_test
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

add_executable(json-stream_test stream_test.cc)
target_link_libraries(json-stream_test gtest gtest_main json)
add_test(
  NAME json-stream_test
  COMMAND json-stream_test
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

add_executable(json-macro_test macro_test.cc)
target_link_libraries(json-macro_test gtest gtest_main json)
add_test(
  NAME json-macro_test
  COMMAND json-macro_test
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

add_executable(json-stream_gen_test ${CMAKE_CURRENT_BINARY_DIR}/test_types.cc
                                    stream_gen_test.cc)
target_include_directories(json-stream_gen_test PRIVATE ${CMAKE_BINARY_DIR})
target_link_libraries(json-stream_gen_test gtest gtest_main json)
add_test(
  NAME json-stream_gen_test
  COMMAND json-stream_gen_test
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

add_executable(json-type_registry_test type_registry_test.cc)
target_include_directories(json-type_registry_test PRIVATE ${CMAKE_BINARY_DIR})
target_link_libraries(json-type_registry_test gtest gtest_main json)
add_test(
  NAME json-type_registry_test
  COMMAND json-type_registry_test
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

add_test(
  NAME json-frontend-tests
  COMMAND python frontend_tests.py --exe-path $<TARGET_FILE:json.exe>
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

format_and_lint(
  json-test
  CMakeLists.txt
  builder_test.cc
  lexer_test.cc
  macro_test.cc
  parser_test.cc
  stream_gen_test.cc
  stream_gen_test.h
  stream_test.cc
  type_registry_test.cc
  variant_test.cc
  frontend_tests.py
  test_types.py
  CCDEPENDS ${CMAKE_CURRENT_BINARY_DIR}/test_types.h)
