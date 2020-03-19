@PACKAGE_INIT@
set(_bindir @PACKAGE_CMAKE_INSTALL_BINDIR@)

include(${CMAKE_CURRENT_LIST_DIR}/libtangent-json-targets.cmake)
check_required_components(libtangent-json)

