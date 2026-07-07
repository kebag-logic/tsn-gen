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

#### Stack codec golden tests. Drive the logic-based encode/decode runtime
#### (tsn::Session + the AVTP/AECP modules) against the real protocol corpus
#### and the example stack, so links traffic_gen in addition to the logic lib.
add_executable(stack_codec-test
    stack_codec_test.cpp
)

target_link_libraries(stack_codec-test
    traffic_gen
    protocol_logic
    protocol_parser
    GTest::gtest_main
    ryml::ryml
)

target_compile_definitions(stack_codec-test PRIVATE
    PROTOCOLS_ROOT_DIR="${tsn-gen_SOURCE_DIR}/protocols"
    STACKS_ROOT_DIR="${tsn-gen_SOURCE_DIR}/stacks"
)

gtest_discover_tests(stack_codec-test)

#### Optional: cross-validate a generated frame against Wireshark's ATDECC
#### dissector. Only registered when tshark is installed, so CI without it
#### simply skips this check instead of failing.
find_program(TSHARK_EXECUTABLE tshark)
if(TSHARK_EXECUTABLE)
    add_test(NAME stack_codec_tshark
        COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/validate_with_tshark.sh
                $<TARGET_FILE:packet_gen>
                ${tsn-gen_SOURCE_DIR}/protocols
                ${tsn-gen_SOURCE_DIR}/stacks/aecp_acquire_entity.yaml
                ${TSHARK_EXECUTABLE}
    )
else()
    message(STATUS "tshark not found — skipping stack_codec_tshark validation")
endif()
