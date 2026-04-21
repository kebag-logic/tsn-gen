cmake_minimum_required(VERSION 3.31)
project(protocol_logic VERSION 0.0.1
    DESCRIPTION "Per-protocol logic modules for the tsn-gen stack")

include(GNUInstallDirs)

set(PROTO_LOGIC_SRCS
    src/ethernet_frame_logic.cpp
)

set(PROTO_LOGIC_INC_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/inc
)

add_library(${PROJECT_NAME} SHARED
    ${PROTO_LOGIC_SRCS}
)

target_link_libraries(${PROJECT_NAME}
    protocol_parser
)

target_include_directories(${PROJECT_NAME} PUBLIC
    $<BUILD_INTERFACE:${PROTO_LOGIC_INC_DIRS}>
    $<INSTALL_INTERFACE:include/protocol-logic>
)

target_compile_options(${PROJECT_NAME} PUBLIC
    $<$<CONFIG:Debug>:-O0 -g3>
    $<$<CONFIG:Release>:-O3 -g3>
    $<$<CONFIG:MinSizeRel>:-Os -g3>
    $<$<CONFIG:RelWithDebInfo>:-Os -g3>
)

if(${ENABLE_PARSER_TESTS})
    add_subdirectory(tests)
endif()
