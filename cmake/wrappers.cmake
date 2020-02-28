# Wraps add_library and provides additional keyword options that translate
# into additional calls to target_link_libraries, target_set_properties, etc.
# Usage:
# ~~~
#   cc_binary(<target-name> [STATIC|SHARED]
#     SRCS src1.cc src2.cc src3.cc
#     DEPS lib1 lib2 lib3
#
# ~~~
#
# Keyword Arguments:
#
# *SRCS*: a list of source files that go into compilation. These translate to
# the positional arguments of add_library().
#
# *DEPS*: a list of libraries to link against. These should be names that cmake
# understands. They are passed as the positional arguments to
# target_link_libraires
#
# *PKGDEPS*: A list of pkg-config names that are dependencies of this
# executable. They are passed as positional arguments to target_pkg_depends
function(cc_library target_name)
  set(_flags)
  set(_oneargs)
  set(_multiargs SRCS DEPS PKGDEPS)
  cmake_parse_arguments(_args "${_flags}" "${_oneargs}" "${_multiargs}" ${ARGN})

  add_library(${target_name} ${_args_UNPARSED_ARGUMENTS} ${_args_SRCS})
  if(_args_DEPS)
    target_link_libraries(${target_name} PUBLIC ${_args_DEPS})
  endif()
  if(_args_PKGDEPS)
    target_pkg_depends(${target_name} ${_args_PKGDEPS})
  endif()
endfunction()

# Wraps add_executable and provides additional keyword options that translate
# into additional calls to target_link_libraries, target_set_properties, etc.
# Usage:
# ~~~
#   cc_binary(<target-name>
#     SRCS src1.cc src2.cc src3.cc
#     DEPS lib1 lib2 lib3
#
# ~~~
#
# Keyword Arguments:
#
# *SRCS*: a list of source files that go into compilation. These translate to
# the positional arguments of add_executable().
#
# *DEPS*: a list of libraries to link against. These should be names that cmake
# understands. They are passed as the positional arguments to
# target_link_libraires
#
# *PKGDEPS*: A list of pkg-config names that are dependencies of this
# executable. They are passed as positional arguments to target_pkg_depends
function(cc_binary target_name)
  set(_flags)
  set(_oneargs)
  set(_multiargs SRCS DEPS PKGDEPS)
  cmake_parse_arguments(_args "${_flags}" "${_oneargs}" "${_multiargs}" ${ARGN})

  add_executable(${target_name} ${_args_SRCS})
  if(_args_DEPS)
    target_link_libraries(${target_name} PUBLIC ${_args_DEPS})
  endif()
  if(_args_PKGDEPS)
    target_pkg_depends(${target_name} ${_args_PKGDEPS})
  endif()
endfunction()

# Wraps a pair of calls to cc_binary and add_test and provides additional
# keyword options that translate to additional calls to set_property
#
# Usage:
# ~~~
#   cc_test(<cc_binary()-spec>
#     ARGV --flag1 --flag2
#     TEST_DEPS file1 file2
#     WORKING_DIRECTORY <workdir>
#     LABELS label1 label2)
# ~~~
#
# In addition to creating the executable, this command will also create a target
# `run.<test-name>` which can be specified as a target to the build system. When
# specified as a build target, will ensure that all the depenencies are up-to-
# date before executing the test.
#
# The name of the test will match the name of the executable target.
#
# Keyword Arguments:
#
# *ARGV*: a list of additional command line options added to the command used
# for the test
#
# *TEST_DEPS*: additional file-level dependencies required to execute the test,
# such as golden data or resource files.
#
# *WORKING_DIRECTORY*: working directory where to execute the command
#
# *LABELS*: a list of ctest labels pased to set_property(TEST ...)
function(cc_test target_name)
  set(_flags)
  set(_oneargs WORKING_DIRECTORY)
  set(_multiargs ARGV TEST_DEPS LABELS)
  cmake_parse_arguments(_args "${_flags}" "${_oneargs}" "${_multiargs}" ${ARGN})

  set(_cmd $<TARGET_FILE:${target_name}> ${_args_ARGV})
  cc_binary(${target_name} ${_args_UNPARSED_ARGUMENTS})

  if(NOT _args_WORKING_DIRECTORY)
    set(_args_WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
  endif()

  if(_args_TEST_DEPS)
    add_custom_target(testdeps.${target_name} DEPENDS ${TEST_DEPS})
    add_dependencies(testdeps testdeps.${target_name})
  endif()

  add_custom_target(
    run.${target_name}
    COMMAND ${_cmd}
    DEPENDS ${_args_TEST_DEPS}
    WORKING_DIRECTORY ${_args_WORKING_DIRECTORY})

  add_test(
    NAME ${target_name}
    COMMAND ${_cmd}
    WORKING_DIRECTORY ${_args_WORKING_DIRECTORY})

  if(_args_LABELS)
    set_property(
      TEST ${target_name}
      APPEND
      PROPERTY LABELS ${_args_LABELS})
  endif()
endfunction()
