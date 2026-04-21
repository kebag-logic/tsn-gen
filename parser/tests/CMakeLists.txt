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
  gtest_parser-test
  gtest.cpp
)

target_link_libraries(
  gtest_parser-test
  GTest::gtest_main
)

#################### Gen for different tests ########

#### Protocol parser tests
set(PROTOCOL_PARSER_SRCS
  protocol_parser_test.cpp
)

target_link_testlibs(
  protocol_parser-test
  protocol_parser
  "${PROTOCOL_PARSER_SRCS}"
  GTest::gtest_main
  ryml::ryml
)

#### Object serializer tests
set(OBJECT_SERIALIZER_SRCS
  object_serializer_test.cpp
)

target_link_testlibs(
  object_serializer-test
  protocol_parser
  "${OBJECT_SERIALIZER_SRCS}"
  GTest::gtest_main
  ryml::ryml
)

#### Logic registry tests
set(LOGIC_REGISTRY_SRCS
  logic_registry_test.cpp
)

target_link_testlibs(
  logic_registry-test
  protocol_parser
  "${LOGIC_REGISTRY_SRCS}"
  GTest::gtest_main
  ryml::ryml
)

#### Stack builder tests
set(STACK_BUILDER_SRCS
  stack_builder_test.cpp
)

target_link_testlibs(
  stack_builder-test
  protocol_parser
  "${STACK_BUILDER_SRCS}"
  GTest::gtest_main
  ryml::ryml
)

################### Make google discover tests ######
include(GoogleTest)
include(CMakeGoogleDiscoverAllTests)
gtest_discover_test_wtestlibs(protocol_parser-test)
gtest_discover_test_wtestlibs(object_serializer-test)
gtest_discover_test_wtestlibs(logic_registry-test)
gtest_discover_test_wtestlibs(stack_builder-test)
gtest_discover_tests(gtest_parser-test)
