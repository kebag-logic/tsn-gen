# Embedded consumer control: tsn-gen added with add_subdirectory, as in
# docs/mid-level/embedding-the-library.md. tsn-gen is not the top-level
# project here, so ENABLE_PARSER_TESTS defaults OFF.
cmake_minimum_required(VERSION 3.21)
project(tsngen_embedded_consumer LANGUAGES CXX)

if(NOT DEFINED TSN_GEN_SOURCE_DIR)
    message(FATAL_ERROR "set TSN_GEN_SOURCE_DIR")
endif()
add_subdirectory(${TSN_GEN_SOURCE_DIR} tsn-gen)

add_executable(app main.cpp)
target_link_libraries(app PRIVATE tsn::traffic_gen tsn::protocol_logic)
target_compile_definitions(app PRIVATE
    PROTOCOLS_DIR="${TSN_GEN_SOURCE_DIR}/protocols"
    STACK_FILE="${TSN_GEN_SOURCE_DIR}/stacks/aecp_acquire_entity.yaml")
