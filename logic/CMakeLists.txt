set(PROTO_LOGIC_SRCS
    src/ethernet_frame_logic.cpp
    src/avtp_control_logic.cpp
    src/aecp_aem_logic.cpp
)

set(PROTO_LOGIC_INC_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/inc
)

# Kept SHARED regardless of BUILD_SHARED_LIBS: REGISTER_LOGIC relies on
# static initialisers running at load time, which a static archive only
# guarantees with --whole-archive (see tsn/logic_registry.h).
add_library(protocol_logic SHARED
    ${PROTO_LOGIC_SRCS}
)
add_library(tsn::protocol_logic ALIAS protocol_logic)

target_link_libraries(protocol_logic PUBLIC
    protocol_parser
)

target_include_directories(protocol_logic PUBLIC
    $<BUILD_INTERFACE:${PROTO_LOGIC_INC_DIRS}>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

set_target_properties(protocol_logic PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 0
    EXPORT_NAME protocol_logic
)

target_compile_options(protocol_logic PRIVATE
    $<$<CONFIG:Debug>:-O0 -g3>
    $<$<CONFIG:Release>:-O3 -g3>
    $<$<CONFIG:MinSizeRel>:-Os -g3>
    $<$<CONFIG:RelWithDebInfo>:-Os -g3>
)

install(TARGETS protocol_logic
    EXPORT tsn-genTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(DIRECTORY inc/tsn
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

if(ENABLE_PARSER_TESTS)
    add_subdirectory(tests)
endif()
