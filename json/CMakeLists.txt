set(json_sources
    builder.cc
    item.cc
    json.cc
    pipeline.cc
    stream.cc
    stream_std.cc
    variant.cc)

add_library(json STATIC ${json_sources})
target_link_libraries(json PUBLIC fmt re2)
target_pkg_depends(json libglog)

add_executable(jjson.exe main.cc)
target_link_libraries(jjson.exe PUBLIC argue json)
set_target_properties(jjson.exe PROPERTIES OUTPUT_NAME jjson)

set(json_files
    builder.h
    item.h
    json.h
    pipeline.h
    stream.h
    stream_tpl.h
    stream_std.h
    stream_std_tpl.h
    stream_macros.h
    stream_tpl.h
    variant.h
    ${json_sources})

format_and_lint(json
                ${json_files}
                CMakeLists.txt
                doc/CMakeLists.txt
                test/CMakeLists.txt)

add_subdirectory(test)
add_subdirectory(doc)
