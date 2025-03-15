cmake_minimum_required(VERSION 3.31)
cmake_policy(VERSION 3.31)

##################### BASE PROJECT INFO AND CONFIGURATIONS
project(tsn_gen)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -Werror -pedantic")

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

##################### OPTIONS

option(ENABLE_PARSER_TESTS "Enable Parser Unit Tests" ON)
option(CONFIG "Build config, Release, Debug..." Release)

##################### VARS

#################### COMPLIANCE VAR FOR TESTING ####################

set(COMP_COVERAGE_FLAGS "-fprofile-arcs -ftest-coverage")
set(COMP_UBSAN_FLAGS "-static-libasan -fsanitize=undefined")
set(COMP_ASAN_FLAGS "-static-libasan -fsanitize=address")

##################### EXTERNAL DEPENDENCIES

add_subdirectory(external)
add_subdirectory(parser)


#### Normal applicaiton #### 

# add_executable(${PROJECT_NAME} 
#     main.cpp
#     )

# target_link_libraries(${PROJECT_NAME} 
#     rpyl
#     )

