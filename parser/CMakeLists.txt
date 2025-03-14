cmake_minimum_required(VERSION 3.31)
project(protocol_parser VERSION 0.0.1
	DESCRIPTION "The parser library to generate tsnpp stack")

##################### MODULES   ####################
include(GNUInstallDirs)

##################### LIBRARY   ####################
add_library(protocol_parser
	src/protocol-parser.cpp
)

target_include_directories(protocol_parser PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/inc/>
  $<INSTALL_INTERFACE:include/protocol-parser>
)

##################### UNIT TESTS ####################

# The Parses
if (${ENABLE_PARSER_TESTS})
	add_subdirectory(tests)
endif(${ENABLE_PARSER_TESTS})
