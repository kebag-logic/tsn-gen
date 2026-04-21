enable_testing()

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
)

add_compile_options(
    -DLOGIC_TESTS_RES_PATH="${CMAKE_CURRENT_SOURCE_DIR}/test_resources"
)

#### Ethernet frame logic tests. Links the shared protocol_logic so the
#### REGISTER_LOGIC static init runs, plus protocol_parser for the
#### StackBuilder machinery. Sanitized variants are already covered at
#### the parser layer.
add_executable(ethernet_frame_logic-test
    ethernet_frame_logic_test.cpp
)

target_link_libraries(ethernet_frame_logic-test
    protocol_logic
    protocol_parser
    GTest::gtest_main
    ryml::ryml
)

include(GoogleTest)
gtest_discover_tests(ethernet_frame_logic-test)
