# NOTE(josh): don't build gtest if it's not part of this sparse checkout
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/googletest)
  message(Enabling third_party:googletest for this checkout)
  add_library(gtest STATIC googletest/src/gtest-all.cc)
  add_library(gtest-shared SHARED googletest/src/gtest-all.cc)
  set_target_properties(
    gtest gtest-shared PROPERTIES
    OUTPUT_NAME gtest
    DEBUG_POSTFIX d)

  foreach(_target gtest gtest-shared)
    target_include_directories(${_target} SYSTEM PUBLIC googletest/include)
    target_include_directories(${_target} PRIVATE googletest)

    # NOTE(josh): compatible with cmake 3.1.0+
    target_link_libraries(${_target} Threads::Threads)

    # NOTE(josh): compatible with cmake 2.8.12+. target_compile_options(gtest
    # PUBLIC "-pthread") target_link_libraries(gtest PUBLIC
    # ${CMAKE_THREAD_LIBS_INIT})
  endforeach()

  add_library(gtest_main STATIC googletest/src/gtest_main.cc)
  target_include_directories(gtest_main SYSTEM PUBLIC googletest/include)
  target_include_directories(gtest_main PRIVATE googletest)

  # NOTE(josh): compatible with cmake 3.1.0+
  target_link_libraries(gtest_main Threads::Threads)

  # NOTE(josh): compatible with cmake 2.8.12+. target_compile_options(gtest_main
  # PUBLIC "-pthread") target_link_libraries(gtest_main PUBLIC
  # ${CMAKE_THREAD_LIBS_INIT})
endif()

# NOTE(josh): don't build re2 if it's not part of this sparse checkout
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/re2)
  message(Enabling third_party:re2 for this checkout)
  set(re2_sources
      re2/util/rune.cc
      re2/util/strutil.cc
      re2/re2/bitstate.cc
      re2/re2/compile.cc
      re2/re2/dfa.cc
      re2/re2/filtered_re2.cc
      re2/re2/mimics_pcre.cc
      re2/re2/nfa.cc
      re2/re2/onepass.cc
      re2/re2/parse.cc
      re2/re2/perl_groups.cc
      re2/re2/prefilter.cc
      re2/re2/prefilter_tree.cc
      re2/re2/prog.cc
      re2/re2/re2.cc
      re2/re2/regexp.cc
      re2/re2/set.cc
      re2/re2/simplify.cc
      re2/re2/stringpiece.cc
      re2/re2/tostring.cc
      re2/re2/unicode_casefold.cc
      re2/re2/unicode_groups.cc)

  add_library(re2 STATIC ${re2_sources})
  add_library(re2-shared SHARED ${re2_sources})
  set_target_properties(
    re2 re2-shared PROPERTIES
    OUTPUT_NAME re2
    DEBUG_POSTFIX d)
  foreach(_target re2 re2-shared)
    target_link_libraries(${_target} Threads::Threads)
  endforeach()

endif()

# NOTE(josh): don't build libfmt if it's not part of this sparse checkout
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/fmt)
  add_subdirectory(fmt)
endif()

if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/CImg)
  add_library(cimg INTERFACE)
  target_include_directories(cimg INTERFACE CImg)
endif()

if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/phash)
  set(phash_sources
      phash/src/pHash.cpp)
  add_library(phash STATIC ${phash_sources})
  add_library(phash-shared SHARED ${phash_sources})
  set_target_properties(
      phash phash-shared PROPERTIES
      OUTPUT_NAME phash
      DEBUG_POSTFIX d)
  foreach(_target phash phash-shared)
    target_include_directories(${_target} PUBLIC phash/include)
    target_include_directories(${_target} PRIVATE phash/private)
    target_link_libraries(${_target} PUBLIC cimg Threads::Threads)
    target_pkg_depends(${_target} libpng)
  endforeach()
endif()
