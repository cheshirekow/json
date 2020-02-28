set(_sources linked.cc redblack.cc stack_trace.cc string_util.cc)

add_library(util STATIC ${_sources})
add_library(util-shared SHARED ${_sources})
format_and_lint(
  util
  binary_literal.h
  bitset.h
  bitset_test.cc
  CMakeLists.txt
  container_of.h
  fixed_string_stream.h
  fixed_string_stream_test.cc
  hash_tpl.h
  inline_math.h
  linked.cc
  linked.h
  linked.hpp
  linked_test.cc
  null_out.h
  range.hpp
  redblack.cc
  redblack.h
  redblack_test.cc
  set_operations.hpp
  stack_trace.h
  stack_trace.cc
  string_util.h
  string_util.cc
  svg_artist.cc
  svg_artist.h
  timespec.h
  type_string.h
  type_string_test.cc)

add_executable(util-bitset_test bitset_test.cc)
target_link_libraries(util-bitset_test gtest gtest_main util)
add_test(
  NAME util-bitset_test
  COMMAND util-bitset_test
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

add_executable(util-linked_test linked_test.cc)
target_link_libraries(util-linked_test gtest gtest_main util)
add_test(
  NAME util-linked_test
  COMMAND util-linked_test
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

add_executable(util-redblack_test redblack_test.cc)
target_link_libraries(util-redblack_test gtest gtest_main util)
add_test(
  NAME util-redblack_test
  COMMAND util-redblack_test
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test_target(util-redblack_test)

add_executable(util-type_string_test type_string_test.cc)
target_link_libraries(util-type_string_test gtest gtest_main util)
add_test(
  NAME util-type_string_test
  COMMAND util-type_string_test
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
set_property(
  TEST util-type_string_test
  APPEND
  PROPERTY LABELS CI-DISABLED)

add_library(svg_artist STATIC svg_artist.cc)
target_pkg_depends(svg_artist libglog eigen3)
target_link_libraries(svg_artist PUBLIC fmt)
