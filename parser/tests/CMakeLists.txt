enable_testing()

add_executable(
  gtest_parser
  gtest.cpp
)

target_link_libraries(
  gtest_parser
  GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(gtest_parser)
