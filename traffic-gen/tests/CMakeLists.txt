#################### GTest ENABLE ##################
enable_testing()

##################### MODULES   ####################
include(CMakeLinkToTestlibs)

#################### Add compile option ###########

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
)

add_compile_options(
  -DPARSER_TESTS_RES_PATH="${CMAKE_CURRENT_SOURCE_DIR}/test_resources"
)

#################### List of test execs ############

add_executable(
  gtest_traffic_gen-test
  gtest.cpp
)

target_link_libraries(
  gtest_traffic_gen-test
  GTest::gtest_main
)

#################### Gen for different tests ########

#### First case the traffic_gen
set(TRAFFIC_GEN_PARSER_SRCS
  traffic_gen_test.cpp
)

target_link_testlibs(
  traffic_gen-test
  traffic_gen
  "${TRAFFIC_GEN_PARSER_SRCS}"
  GTest::gtest_main
  ryml::ryml
)

target_link_testlibs(
  object_serializer-test
  traffic_gen
  "${TRAFFIC_GEN_PARSER_SRCS}"
  GTest::gtest_main
  ryml::ryml
)

################### Make google discover tests ######
include(GoogleTest)
include(CMakeGoogleDiscoverAllTests)
gtest_discover_test_wtestlibs(traffic_gen-test)
gtest_discover_tests(gtest_traffic_gen-test)

