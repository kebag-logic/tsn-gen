#################### GTest ENABLE ##################
enable_testing()

##################### MODULES   ####################
include(CMakeLinkToTestlibs)

#################### Compile definitions ############
add_compile_options(
  -DTRAFFIC_GEN_TESTS_RES_PATH="${tsn-gen_SOURCE_DIR}/parser/tests/test_resources"
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

#################### Traffic generator tests ########
set(TRAFFIC_GEN_SRCS
  traffic_gen_test.cpp
)

target_link_testlibs(
  traffic_gen-test
  traffic_gen
  "${TRAFFIC_GEN_SRCS}"
  GTest::gtest_main
  ryml::ryml
)

#################### ADP fuzzer tests ###############
set(ADP_FUZZ_SRCS
  adp_fuzz_test.cpp
)

target_link_testlibs(
  adp_fuzz-test
  traffic_gen
  "${ADP_FUZZ_SRCS}"
  GTest::gtest_main
  ryml::ryml
)

#################### 802.1AS PTP flag tests #########
# Grades the shipped protocols/ corpus itself (PROTOCOLS_ROOT_DIR, as the
# logic stack_codec-test does) rather than a fixture copy, so the models the
# TX flag oracle asserts are the ones a consumer loads.
set(PTP_FLAGS_SRCS
  ptp_flags_test.cpp
)

target_link_testlibs(
  ptp_flags-test
  traffic_gen
  "${PTP_FLAGS_SRCS}"
  GTest::gtest_main
  ryml::ryml
)

# target_link_testlibs emits the plain, ASan and UBSan executables.
foreach(_target ptp_flags-test ptp_flags-test_ASAN ptp_flags-test_UBSAN)
  target_compile_definitions(${_target} PRIVATE
    PROTOCOLS_ROOT_DIR="${tsn-gen_SOURCE_DIR}/protocols"
  )
endforeach()

################### GTest discovery #################
include(GoogleTest)
include(CMakeGoogleDiscoverAllTests)
gtest_discover_test_wtestlibs(traffic_gen-test)
gtest_discover_test_wtestlibs(adp_fuzz-test)
gtest_discover_test_wtestlibs(ptp_flags-test)

################### Corpus conformance ##############
# The shipped protocols/ and stacks/ must pass `packet_gen --validate`, so a
# non-conforming file added later fails CI. Uses the just-built CLI.
add_test(NAME corpus_validates
    COMMAND packet_gen
            --yaml-dir ${tsn-gen_SOURCE_DIR}/protocols
            --stack-file ${tsn-gen_SOURCE_DIR}/stacks/aecp_acquire_entity.yaml
            --validate
)
