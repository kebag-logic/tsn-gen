enable_testing()

add_executable(
  gtest_parser-test
  gtest.cpp
)

add_executable(
  protocol_parser-test
  protocol-parser-test.cpp
)

target_link_libraries(
  protocol_parser-test
  protocol_parser
  GTest::gtest_main
)

target_link_libraries(
  gtest_parser-test
  GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(gtest_parser-test)
gtest_discover_tests(protocol_parser-test)
