#################### GTest ENABLE ##################
enable_testing()

##################### MODULES   ####################
include(CMakeLinkToTestlibs)


#################### List of test execs ############


add_executable(
  gtest_parser-test
  gtest.cpp
)

target_link_libraries(
  gtest_parser-test
  GTest::gtest_main
)

#################### Gen for different tests ########

#### First case the protocol parser
set(PROTOCOL_PARSER_SRCS
	protocol-parser-test.cpp
)

target_link_testlibs(
  protocol_parser-test
  protocol_parser
  "${PROTOCOL_PARSER_SRCS}"
  GTest::gtest_main
)

################### Make google discover tests ######
include(GoogleTest)
include(CMakeGoogleDiscoverAllTests)
gtest_discover_test_wtestlibs(protocol_parser-test)
gtest_discover_tests(gtest_parser-test)
