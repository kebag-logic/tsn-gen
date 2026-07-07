cmake_minimum_required(VERSION 3.21...3.31)

##################### BASE PROJECT INFO AND CONFIGURATIONS
project(tsn-gen VERSION 0.1.0 LANGUAGES CXX
    DESCRIPTION "YAML-driven TSN protocol traffic generation and debugging toolkit")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Installed binaries/libraries locate their co-installed dependencies via a
# relative RPATH, so `packet_gen` runs from any prefix without
# LD_LIBRARY_PATH: $ORIGIN for a library's sibling libs, $ORIGIN/../lib for
# a bin/ executable reaching lib/.
if(NOT DEFINED CMAKE_INSTALL_RPATH)
    set(CMAKE_INSTALL_RPATH "$ORIGIN;$ORIGIN/../lib")
endif()
set(CMAKE_BUILD_RPATH_USE_ORIGIN ON)

# Strict flags are opt-in for embedders: only enforced when tsn-gen is the
# top-level project so a consumer's warnings never break their build here.
if(PROJECT_IS_TOP_LEVEL)
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -Werror -pedantic")
endif()

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

##################### OPTIONS

option(ENABLE_PARSER_TESTS "Enable unit tests and sanitizer library variants"
    ${PROJECT_IS_TOP_LEVEL})
option(ENABLE_SYSTEMC_SIM
    "Build the ns-3 + Verilator co-simulation (requires SystemC and ns-3 with --enable-systemc)"
    OFF)

#################### COMPLIANCE VAR FOR TESTING ####################

set(COMP_COVERAGE_FLAGS "")
set(COMP_UBSAN_FLAGS "-fsanitize=undefined")
set(COMP_ASAN_FLAGS "-fsanitize=address")

##################### TESTING ##############################

if(ENABLE_PARSER_TESTS)
    # Must be enabled here (not only in subdirectories) so that a bare
    # `ctest --test-dir build` from the top level discovers every test.
    enable_testing()
endif()

##################### EXTERNAL DEPENDENCIES

add_subdirectory(external)

#################### COMPONENTS ############################

add_subdirectory(parser)
add_subdirectory(logic)
add_subdirectory(traffic-gen)

if(ENABLE_SYSTEMC_SIM)
    add_subdirectory(sim)
endif()

##################### MAN PAGE ##############################

install(FILES docs/man/packet_gen.1
    DESTINATION ${CMAKE_INSTALL_MANDIR}/man1
)

##################### PROTOCOL DEFINITIONS ##################

# The curated YAML corpus is part of the product: install it so other
# projects can point --yaml-dir / ProtocolParser at a stable location.
install(DIRECTORY protocols/
    DESTINATION ${CMAKE_INSTALL_DATADIR}/tsn-gen/protocols
    FILES_MATCHING PATTERN "*.yaml"
)

# Example stack definitions consumed by packet_gen --stack-file.
install(DIRECTORY stacks/
    DESTINATION ${CMAKE_INSTALL_DATADIR}/tsn-gen/stacks
    FILES_MATCHING PATTERN "*.yaml"
)

# JSON Schema for protocol/stack YAML — the formal DSL contract, usable by
# editors and external validators (packet_gen --validate enforces it too).
install(DIRECTORY docs/schema/
    DESTINATION ${CMAKE_INSTALL_DATADIR}/tsn-gen/schema
    FILES_MATCHING PATTERN "*.json"
)

##################### CMAKE PACKAGE (find_package support) ##

install(EXPORT tsn-genTargets
    NAMESPACE tsn::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/tsn-gen
)

configure_package_config_file(
    cmake/tsn-genConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/tsn-genConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/tsn-gen
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/tsn-genConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    # Pre-1.0: a minor bump may break API, so only same-minor is compatible.
    COMPATIBILITY SameMinorVersion
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/tsn-genConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/tsn-genConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/tsn-gen
)
