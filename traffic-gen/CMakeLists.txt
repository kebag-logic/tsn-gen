cmake_minimum_required(VERSION 3.31)
project(traffic_gen VERSION 0.0.1
	DESCRIPTION "The parser library to generate tsnpp stack")

##################### MODULES   ####################
include(GNUInstallDirs)
include(CMakeGenTestingLibraries)

#################### SOURCES	####################

set(TRAFFIC_GEN_PARSER_SRCS 
	src/traffic_gen.cpp
)

set(TRAFFIC_GEN_PARSER_INC_DIRS
	${CMAKE_CURRENT_SOURCE_DIR}/inc
)

##################### LIBRARY   ####################

add_library(${PROJECT_NAME} SHARED
	${TRAFFIC_GEN_PARSER_SRCS}
)

target_include_directories(${PROJECT_NAME} PUBLIC
  $<BUILD_INTERFACE:${TRAFFIC_GEN_PARSER_INC_DIRS}>
  $<INSTALL_INTERFACE:include/${PROJECT_NAME}/>
)

if (${ENABLE_PARSER_TESTS})
generate_test_libs(
	${PROJECT_NAME}
	"${TRAFFIC_GEN_PARSER_INC_DIRS}"
	${TRAFFIC_GEN_PARSER_SRCS}
)
endif (${ENABLE_PARSER_TESTS})


##################### OPTIONS   ####################

target_compile_options($PROJECT_NAME} PUBLIC
	$<$<CONFIG:Debug>:-O0 -g3>
	$<$<CONFIG:Release>:-O3 -g0>
	$<$<CONFIG:MinSizeRel>:-Os -g0>
	$<$<CONFIG:RelWithDebInfo>:-Os -g3>
)

##################### UNIT TESTS ####################

# The TESTS for Traffic Gen
if (${ENABLE_TRAFFIC_GEN_TESTS})
	add_subdirectory(tests)
endif(${ENABLE_TRAFFIC_GEN_TESTS})

