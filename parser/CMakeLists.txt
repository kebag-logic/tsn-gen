cmake_minimum_required(VERSION 3.31)
project(protocol_parser VERSION 0.0.1
	DESCRIPTION "The parser library to generate tsnpp stack")

##################### MODULES   ####################
include(GNUInstallDirs)
include(CMakeGenTestingLibraries)

#################### SOURCES	####################

set(PROTO_PARSER_SRCS 
	src/protocol_parser.cpp
	src/protocol.cpp
	src/db_proto_if_impl.cpp
	src/db_proto_impl.cpp
	src/db_var_impl.cpp
)

set(PROTO_PARSER_INC_DIRS
	${CMAKE_CURRENT_SOURCE_DIR}/inc
)

##################### LIBRARY   ####################

add_library(${PROJECT_NAME} SHARED
	${PROTO_PARSER_SRCS}
)
target_link_libraries(${PROJECT_NAME}
	ryml::ryml
)

target_include_directories(protocol_parser PUBLIC
  $<BUILD_INTERFACE:${PROTO_PARSER_INC_DIRS}>
  $<INSTALL_INTERFACE:include/protocol-parser>
)

if (${ENABLE_PARSER_TESTS})
generate_test_libs(
	${PROJECT_NAME}
	"${PROTO_PARSER_INC_DIRS}"
	${PROTO_PARSER_SRCS}
)
endif (${ENABLE_PARSER_TESTS})


##################### OPTIONS   ####################

target_compile_options(protocol_parser PUBLIC
	$<$<CONFIG:Debug>:-O0 -g3>
	$<$<CONFIG:Release>:-O3 -g3>
	$<$<CONFIG:MinSizeRel>:-Os -g3>
	$<$<CONFIG:RelWithDebInfo>:-Os -g3>
)

##################### UNIT TESTS ####################

# The Parses
if (${ENABLE_PARSER_TESTS})
	add_subdirectory(tests)
endif(${ENABLE_PARSER_TESTS})
