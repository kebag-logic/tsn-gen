##################### MODULES   ####################
include(CMakeGenTestingLibraries)

#################### SOURCES	####################

set(PROTO_PARSER_SRCS
	src/log.cpp
	src/validator.cpp
	src/protocol_parser.cpp
	src/protocol.cpp
	src/db_proto_if_impl.cpp
	src/db_proto_impl.cpp
	src/db_proto_service_impl.cpp
	src/db_var_impl.cpp
	src/logic_registry.cpp
	src/stack.cpp
	src/stack_builder.cpp
)

set(PROTO_PARSER_INC_DIRS
	${CMAKE_CURRENT_SOURCE_DIR}/inc
)

##################### LIBRARY   ####################

add_library(protocol_parser SHARED
	${PROTO_PARSER_SRCS}
)
add_library(tsn::protocol_parser ALIAS protocol_parser)

# ryml is PUBLIC because installed headers (tsn/protocol.h) use ryml types.
target_link_libraries(protocol_parser PUBLIC
	ryml::ryml
)

target_include_directories(protocol_parser PUBLIC
  $<BUILD_INTERFACE:${PROTO_PARSER_INC_DIRS}>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

set_target_properties(protocol_parser PROPERTIES
	VERSION ${PROJECT_VERSION}
	SOVERSION 0
	EXPORT_NAME protocol_parser
)

if(ENABLE_PARSER_TESTS)
generate_test_libs(
	protocol_parser
	"${PROTO_PARSER_INC_DIRS}"
	${PROTO_PARSER_SRCS}
)

# The generated variants do not inherit the main target's link interface;
# they need ryml's usage requirements (include dirs) explicitly.
foreach(_variant ubsan asan)
	target_link_libraries(protocol_parser_${_variant} ryml::ryml)
endforeach()
endif()


##################### OPTIONS   ####################

# PRIVATE: optimisation/debug levels must not leak into consumers.
target_compile_options(protocol_parser PRIVATE
	$<$<CONFIG:Debug>:-O0 -g3>
	$<$<CONFIG:Release>:-O3 -g3>
	$<$<CONFIG:MinSizeRel>:-Os -g3>
	$<$<CONFIG:RelWithDebInfo>:-Os -g3>
)

##################### INSTALL   ####################

install(TARGETS protocol_parser
	EXPORT tsn-genTargets
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(DIRECTORY inc/tsn
	DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

##################### UNIT TESTS ####################

if(ENABLE_PARSER_TESTS)
	add_subdirectory(tests)
endif()
