set(json_sources
    builder.cc
    item.cc
    json.cc
    parse.cc
    pipeline.cc
    type_registry.cc
    util.cc
    variant.cc)

add_library(json STATIC ${json_sources})
target_link_libraries(json PUBLIC fmt re2 util)

add_library(json-shared SHARED ${json_sources})
target_link_libraries(json-shared PUBLIC fmt-shared re2-shared util-shared)

foreach(_target json json-shared)
  target_pkg_depends(${_target} libglog)
endforeach()

add_executable(json.exe main.cc)
target_link_libraries(json.exe PUBLIC argue json)
set_target_properties(json.exe PROPERTIES OUTPUT_NAME json)

set(json_files
    builder.h
    item.h
    json.h
    parse.h
    pipeline.h
    stream_macros.h
    type_registry.h
    util.h
    variant.h
    ${json_sources})

format_and_lint(
  json
  ${json_files}
  CMakeLists.txt
  doc/CMakeLists.txt
  doc/conf.py
  json_gen.py
  main.cc
  EXCLUDE "test/.*")

add_subdirectory(test)
add_subdirectory(doc)
