add_executable(json-lexer_test lexer_test.cc)
target_link_libraries(json-lexer_test gtest gtest_main json)
add_test(NAME json-lexer_test
         COMMAND json-lexer_test
         WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

add_executable(json-parser_test parser_test.cc)
target_link_libraries(json-parser_test gtest gtest_main json)
add_test(NAME json-parser_test
         COMMAND json-parser_test
         WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

add_executable(json-builder_test builder_test.cc)
target_link_libraries(json-builder_test gtest gtest_main json)
add_test(NAME json-builder_test
         COMMAND json-builder_test
         WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

add_executable(json-variant_test variant_test.cc)
target_link_libraries(json-variant_test gtest gtest_main json)
add_test(NAME json-variant_test
         COMMAND json-variant_test
         WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

add_executable(json-stream_test stream_test.cc)
target_link_libraries(json-stream_test gtest gtest_main json)
add_test(NAME json-stream_test
         COMMAND json-stream_test
         WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

add_executable(json-macro_test macro_test.cc)
target_link_libraries(json-macro_test gtest gtest_main json)
add_test(NAME json-macro_test
         COMMAND json-macro_test
         WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
