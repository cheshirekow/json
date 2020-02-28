set(json_sources
    builder.cc
    emit.cc
    emit_std.cc
    item.cc
    json.cc
    parse.cc
    parse_std.cc
    pipeline.cc
    stream.cc
    stream_std.cc
    type_registry.cc
    util.cc
    variant.cc)

add_library(json STATIC ${json_sources})
target_link_libraries(json PUBLIC fmt re2)

add_library(json-shared SHARED ${json_sources})
target_link_libraries(json-shared PUBLIC fmt-shared re2-shared)

foreach(_target json json-shared)
  target_pkg_depends(${_target} libglog)
endforeach()

add_executable(json.exe main.cc)
target_link_libraries(json.exe PUBLIC argue json)
set_target_properties(json.exe PROPERTIES OUTPUT_NAME json)

set(json_files
    builder.h
    emit.h
    emit_std.h
    item.h
    json.h
    json_std.h
    parse.h
    parse_std.h
    pipeline.h
    stream.h
    stream_macros.h
    stream_std.h
    stream_tpl.h
    type_registry.h
    util.h
    variant.h
    ${json_sources})

format_and_lint(json ${json_files} CMakeLists.txt doc/CMakeLists.txt
                test/CMakeLists.txt)

add_subdirectory(test)
add_subdirectory(doc)
