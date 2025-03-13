cmake_minimum_required(VERSION 3.31)
cmake_policy(VERSION 3.31)

##################### BASE PROJECT INFO AND CONFIGURATIONS
project(tsn_gen)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS_RELEASE "-O3 -Werror -pedantic")


##################### OPTIONS

option(ENABLE_PARSER_TESTS "Enable Parser Unit Tests" ON)

##################### EXTERNAL DEPENDENCIES

add_subdirectory(parser)
add_subdirectory(external)

##################### UNIT TESTS

# The Parses
if (${ENABLE_PARSER_TESTS})
    add_subdirectory(parser/tests)
endif(${ENABLE_PARSER_TESTS})

#### Normal applicaiton #### 

#target_compile_options(${PROJECT_NAME} PUBLIC
#    $<$<CMAKE_BUILD_TYPE:Debug>:-O0 -g3>
#    $<$<CMAKE_BUILD_TYPE:Release>:-O3 -g0>
#    $<$<CMAKE_BUILD_TYPE:MinSizeRel>:-Os -g0>
#    $<$<CMAKE_BUILD_TYPE:RelWithDebInfo>:-Os -g3>
#    )
# add_executable(${PROJECT_NAME} 
#     main.cpp
#     )

# target_link_libraries(${PROJECT_NAME} 
#     rpyl
#     )

