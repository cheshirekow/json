set(json_sources
    builder.cc
    item.cc
    json.cc
    pipeline.cc
    stream.cc
    stream_std.cc
    variant.cc)

add_library(json STATIC ${json_sources})
target_link_libraries(json re2)

add_executable(json.exe main.cc)
target_link_libraries(json.exe argue json)
set_target_properties(json.exe PROPERTIES OUTPUT_NAME json)

set(json_files
    builder.h
    item.h
    json.h
    pipeline.h
    stream.h
    stream_std.h
    stream_macros.h
    variant.h
    ${json_sources})

add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/json_format.stamp
                   COMMAND clang-format -style file -i ${json_files}
                   COMMAND python -Bm cmake_format -i CMakeLists.txt
                           doc/CMakeLists.txt test/CMakeLists.txt
                   COMMAND touch ${CMAKE_CURRENT_BINARY_DIR}/json_format.stamp
                   DEPENDS ${json_files}
                   WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
add_custom_target(json_format
                  DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/json_format.stamp)
add_dependencies(format json_format)

add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/json_lint.stamp
                   COMMAND cpplint ${json_files}
                   COMMAND touch ${CMAKE_CURRENT_BINARY_DIR}/json_lint.stamp
                   DEPENDS ${json_files}
                   WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
add_custom_target(json_lint DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/json_lint.stamp)

# NOTE(josh): format before lint-check, avoid formatting lint
add_dependencies(json_lint json_format)
add_dependencies(lint json_lint)

add_subdirectory(test)
add_subdirectory(doc)