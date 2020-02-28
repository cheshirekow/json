set(argue_headers argue.h argue_glog.h)
set(argue_sources argue.cc argue_glog.cc)

set(argue_files
    ${argue_headers}
    ${aruge_sources}
    demo.cc
    test/arg_test.cc
    test/assert_test.cc
    test/parse_test.cc
    test/util_test.cc)

add_library(argue STATIC ${argue_sources})
target_pkg_depends(argue libglog)
target_link_libraries(argue PUBLIC fmt)

add_executable(argue-demo demo.cc)
target_link_libraries(argue-demo argue)

format_and_lint(argue ${argue_files})

add_subdirectory(doc)

add_executable(argue-assert_test test/assert_test.cc)
target_link_libraries(argue-assert_test argue gtest gtest_main)
add_test(NAME argue-assert_test
         COMMAND argue-assert_test
         WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
set_property(TEST argue-assert_test APPEND PROPERTY LABELS CI-DISABLED)

add_executable(argue-arg_test test/arg_test.cc)
target_link_libraries(argue-arg_test argue gtest gtest_main)
add_test(NAME argue-arg_test
         COMMAND argue-arg_test
         WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

add_executable(argue-parse_test test/parse_test.cc)
target_link_libraries(argue-parse_test argue gtest gtest_main)
add_test(NAME argue-parse_test
         COMMAND argue-parse_test
         WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

add_executable(argue-util_test test/util_test.cc)
target_link_libraries(argue-util_test argue gtest gtest_main)
add_test(NAME argue-util_test
         COMMAND argue-util_test
         WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

add_test(NAME argue-execution_test
         COMMAND python ${CMAKE_CURRENT_SOURCE_DIR}/test/execution_tests.py
                 --exe-path $<TARGET_FILE:argue-demo>
          WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
set_property(TEST argue-execution_test APPEND PROPERTY LABELS CI-DISABLED)
