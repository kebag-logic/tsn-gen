##################### MODULES   ####################
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
    src/session.cpp
)

set(TRAFFIC_GEN_INC_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/inc
)

#################### LIBRARY    ####################
add_library(traffic_gen SHARED
    ${TRAFFIC_GEN_SRCS}
)
add_library(tsn::traffic_gen ALIAS traffic_gen)

target_include_directories(traffic_gen PUBLIC
    $<BUILD_INTERFACE:${TRAFFIC_GEN_INC_DIRS}>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

# PUBLIC: traffic-gen headers include tsn/protocol_parser.h and friends.
target_link_libraries(traffic_gen PUBLIC
    protocol_parser
)

set_target_properties(traffic_gen PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 0
    EXPORT_NAME traffic_gen
)

#################### SANITISER VARIANTS ############
if(ENABLE_PARSER_TESTS)
    generate_test_libs(
        traffic_gen
        "${TRAFFIC_GEN_INC_DIRS}"
        ${TRAFFIC_GEN_SRCS}
    )

    foreach(_variant ubsan asan)
        target_link_libraries(traffic_gen_${_variant}
            protocol_parser_${_variant}
            ryml::ryml
        )
        target_include_directories(traffic_gen_${_variant} PUBLIC
            $<BUILD_INTERFACE:${TRAFFIC_GEN_INC_DIRS}>
        )
    endforeach()
endif()

#################### COMPILE OPTIONS ###############
# PRIVATE: optimisation/debug levels must not leak into consumers.
target_compile_options(traffic_gen PRIVATE
    $<$<CONFIG:Debug>:-O0 -g3>
    $<$<CONFIG:Release>:-O3 -g3>
    $<$<CONFIG:MinSizeRel>:-Os -g3>
    $<$<CONFIG:RelWithDebInfo>:-Os -g3>
)

#################### CLI TOOL   ####################
add_executable(packet_gen
    tools/packet_gen_main.cpp
)

# protocol_logic is linked so its REGISTER_LOGIC static initialisers run and
# the stack runtime (--stack-file) can resolve modules by name. Whole-archive
# is only needed for a static build; the library is SHARED here.
target_link_libraries(packet_gen
    traffic_gen
    protocol_logic
)

#################### INSTALL    ####################
install(TARGETS traffic_gen packet_gen
    EXPORT tsn-genTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(DIRECTORY inc/tsn
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

#################### UNIT TESTS ####################
if(ENABLE_PARSER_TESTS)
    add_subdirectory(tests)
endif()
