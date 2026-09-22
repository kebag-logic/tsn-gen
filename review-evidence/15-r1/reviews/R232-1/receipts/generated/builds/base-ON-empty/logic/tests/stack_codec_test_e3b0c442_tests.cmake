add_test([=[StackCodec.FrameHasExpectedSize]=]  $WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/builds/base-ON-empty/logic/tests/stack_codec-test [==[--gtest_filter=StackCodec.FrameHasExpectedSize]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[StackCodec.FrameHasExpectedSize]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[$WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/base/logic/tests/stack_codec_test.cpp:66]==]
    WORKING_DIRECTORY [==[$WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/builds/base-ON-empty/logic/tests]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[StackCodec.DerivedFieldsAreValid]=]  $WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/builds/base-ON-empty/logic/tests/stack_codec-test [==[--gtest_filter=StackCodec.DerivedFieldsAreValid]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[StackCodec.DerivedFieldsAreValid]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[$WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/base/logic/tests/stack_codec_test.cpp:74]==]
    WORKING_DIRECTORY [==[$WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/builds/base-ON-empty/logic/tests]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[StackCodec.CommandStatusForcedToZero]=]  $WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/builds/base-ON-empty/logic/tests/stack_codec-test [==[--gtest_filter=StackCodec.CommandStatusForcedToZero]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[StackCodec.CommandStatusForcedToZero]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[$WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/base/logic/tests/stack_codec_test.cpp:91]==]
    WORKING_DIRECTORY [==[$WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/builds/base-ON-empty/logic/tests]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[StackCodec.DecodeRoundTripsFields]=]  $WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/builds/base-ON-empty/logic/tests/stack_codec-test [==[--gtest_filter=StackCodec.DecodeRoundTripsFields]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[StackCodec.DecodeRoundTripsFields]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[$WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/base/logic/tests/stack_codec_test.cpp:106]==]
    WORKING_DIRECTORY [==[$WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/builds/base-ON-empty/logic/tests]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[StackCodec.DeterministicForFixedSeed]=]  $WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/builds/base-ON-empty/logic/tests/stack_codec-test [==[--gtest_filter=StackCodec.DeterministicForFixedSeed]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[StackCodec.DeterministicForFixedSeed]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[$WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/base/logic/tests/stack_codec_test.cpp:129]==]
    WORKING_DIRECTORY [==[$WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/builds/base-ON-empty/logic/tests]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
set(stack_codec-test_TESTS [==[StackCodec.FrameHasExpectedSize]==] [==[StackCodec.DerivedFieldsAreValid]==] [==[StackCodec.CommandStatusForcedToZero]==] [==[StackCodec.DecodeRoundTripsFields]==] [==[StackCodec.DeterministicForFixedSeed]==])
