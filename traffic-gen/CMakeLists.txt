cmake_minimum_required(VERSION 3.31)
project(traffic_gen VERSION 0.0.1
    DESCRIPTION "Packet traffic generator driven by parsed protocol definitions")

##################### MODULES   ####################
include(GNUInstallDirs)
include(CMakeGenTestingLibraries)

#################### SOURCES    ####################
set(TRAFFIC_GEN_SRCS
    src/packet_builder.cpp
    src/packet_decoder.cpp
    src/pcap_sender.cpp
    src/pcap_receiver.cpp
    src/raw_socket_sender.cpp
    src/raw_socket_receiver.cpp
    src/verilator_sender.cpp
    src/verilator_receiver.cpp
    src/traffic_generator.cpp
)

set(TRAFFIC_GEN_INC_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/inc
)

#################### LIBRARY    ####################
add_library(${PROJECT_NAME} SHARED
    ${TRAFFIC_GEN_SRCS}
)

target_include_directories(${PROJECT_NAME} PUBLIC
    $<BUILD_INTERFACE:${TRAFFIC_GEN_INC_DIRS}>
    $<INSTALL_INTERFACE:include/traffic-gen>
)

target_link_libraries(${PROJECT_NAME}
    protocol_parser
    ryml::ryml
)

#################### SANITISER VARIANTS ############
if (${ENABLE_PARSER_TESTS})
    generate_test_libs(
        ${PROJECT_NAME}
        "${TRAFFIC_GEN_INC_DIRS}"
        ${TRAFFIC_GEN_SRCS}
    )

    foreach(_variant ubsan asan)
        target_link_libraries(${PROJECT_NAME}_${_variant}
            protocol_parser_${_variant}
            ryml::ryml
        )
        target_include_directories(${PROJECT_NAME}_${_variant} PUBLIC
            $<BUILD_INTERFACE:${TRAFFIC_GEN_INC_DIRS}>
        )
    endforeach()
endif()

#################### COMPILE OPTIONS ###############
target_compile_options(${PROJECT_NAME} PUBLIC
    $<$<CONFIG:Debug>:-O0 -g3>
    $<$<CONFIG:Release>:-O3 -g3>
    $<$<CONFIG:MinSizeRel>:-Os -g3>
    $<$<CONFIG:RelWithDebInfo>:-Os -g3>
)

#################### CLI TOOL   ####################
add_executable(packet_gen
    tools/packet_gen_main.cpp
)

target_include_directories(packet_gen PRIVATE
    ${TRAFFIC_GEN_INC_DIRS}
)

target_link_libraries(packet_gen
    ${PROJECT_NAME}
    protocol_parser
    ryml::ryml
)

install(TARGETS packet_gen
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

#################### UNIT TESTS ####################
if (${ENABLE_PARSER_TESTS})
    add_subdirectory(tests)
endif()
