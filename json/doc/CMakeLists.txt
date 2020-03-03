set(_stagefiles)
set(_stage ${CMAKE_CURRENT_BINARY_DIR}/stage)

configure_file(conf.py ${_stage}/conf.py @ONLY)
list(APPEND _stagefiles ${_stage}/conf.py)

stage_files(
  LIST _stagefiles
  STAGE ${_stage}
  SOURCEDIR ${CMAKE_CURRENT_SOURCE_DIR}
  FILES changelog.rst
        index.rst
        json_program.rst
        low_level.rst
        old_stream.rst
        stream.rst
        release_notes.rst)

stage_files(
  LIST _stagefiles
  STAGE ${_stage}
  SOURCEDIR ${CMAKE_SOURCE_DIR}/doc
  FILES conf.py:conf_common.py sphinx-static/css/cheshire_theme.css)

add_custom_target(stagefiles-json-doc DEPENDS ${_stagefiles})

set(_sphinx_manifest ${_stagefiles})
list(SORT _sphinx_manifest)

# NOTE(josh): cmake 3.10 does not have list(JOIN)
string(REPLACE ";" "\n" _manifest_text "${_sphinx_manifest}")

# NOTE(josh): don't use file(WRITE) because that wont create a dependency on the
# configure step to rerun cmake if the file is missing
configure_file(MANIFEST.in ${CMAKE_CURRENT_BINARY_DIR}/MANIFEST
               NEWLINE_STYLE UNIX)

add_custom_target(
  clean-json-sphinx-stage
  DEPENDS ${_sphinx_manifest} ${CMAKE_CURRENT_BINARY_DIR}/MANIFEST
  COMMAND python -Bm tangent.tooling.clean_stage
          ${CMAKE_CURRENT_BINARY_DIR}/MANIFEST ${_stage}
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  COMMENT "Cleaning cmake_format/doc/stage")

add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/html/index.html
  COMMAND env PYTHONPATH=${CMAKE_SOURCE_DIR} sphinx-build -M html ${_stage}
          ${CMAKE_CURRENT_BINARY_DIR}
  COMMAND touch ${CMAKE_CURRENT_BINARY_DIR}/html/index.html
  DEPENDS clean-json-sphinx-stage ${_sphinx_manifest}
          ${CMAKE_CURRENT_BINARY_DIR}/MANIFEST ${CMAKE_SOURCE_DIR}/doc/conf.py
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  COMMENT "Building sphinx documentation for cmake_format")

add_custom_target(doc-json DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/html/index.html)

add_custom_target(
  showdoc-json
  COMMAND xdg-open ${CMAKE_CURRENT_BINARY_DIR}/html/index.html
  DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/html/index.html)

add_dependencies(doc doc-json)
set_property(GLOBAL APPEND PROPERTY global_doc_files ${_sphinx_manifest})

set_property(DIRECTORY APPEND PROPERTY ADDITIONAL_MAKE_CLEAN_FILES
                                       ${_stagefiles})

set(_deploykey)
if(IS_TRAVIS_CI)
  set(_keystore ${CMAKE_SOURCE_DIR}/tangent/tooling/deploy_keys)
  set(_deploykey ${CMAKE_CURRENT_BINARY_DIR}/libtangent-json-rtd.deploy_key)
  add_custom_command(
    OUTPUT ${_deploykey}
    COMMAND
      # cmake-format: off
      openssl aes-256-cbc
      -K $ENV{encrypted_50897b1cc321_key}
      -iv $ENV{encrypted_50897b1cc321_iv}
      -in ${_keystore}/libtangent-json-rtd.enc
      -out ${_deploykey}
      -d
      # cmake-format: on
    COMMAND chmod 0600 ${_deploykey}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Decrypting libtangent-json-rtd.deploy_key")
endif()

add_custom_target(
  push-json-rtd-repo
  COMMAND
    # cmake-format: off
    python -Bm tangent.tooling.github sync-doc-artifacts
      --doc-repo git@github.com:cheshirekow/libtangent-json-rtd.git
      --repo-dir ${CMAKE_CURRENT_BINARY_DIR}/libtangent-json-rtd.git
      --scratch-tree ${CMAKE_CURRENT_BINARY_DIR}/scratch-tree
      --stage ${_stage}
      --deploy-key "${_deploykey}"
      --tag \$\${TRAVIS_TAG}
    # cmake-format: on
  DEPENDS clean-json-sphinx-stage ${_sphinx_manifest} ${_deploykey}
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  COMMENT "Pushing doc artifacts to the RTD repository")
