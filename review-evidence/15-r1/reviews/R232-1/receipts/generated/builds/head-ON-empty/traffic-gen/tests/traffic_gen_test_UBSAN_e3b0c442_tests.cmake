add_test([=[TrafficGenTest.EmptyInterfaceProducesEmptyPacket]=]  $WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/builds/head-ON-empty/traffic-gen/tests/traffic_gen-test_UBSAN [==[--gtest_filter=TrafficGenTest.EmptyInterfaceProducesEmptyPacket]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[TrafficGenTest.EmptyInterfaceProducesEmptyPacket]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[$WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/head/traffic-gen/tests/traffic_gen_test.cpp:91]==]
    WORKING_DIRECTORY [==[$WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/builds/head-ON-empty/traffic-gen/tests]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[TrafficGenTest.SimpleVarProducesOneByte]=]  $WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/builds/head-ON-empty/traffic-gen/tests/traffic_gen-test_UBSAN [==[--gtest_filter=TrafficGenTest.SimpleVarProducesOneByte]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[TrafficGenTest.SimpleVarProducesOneByte]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[$WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/head/traffic-gen/tests/traffic_gen_test.cpp:101]==]
    WORKING_DIRECTORY [==[$WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/builds/head-ON-empty/traffic-gen/tests]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[TrafficGenTest.SimpleVarValueInExpectedSet]=]  $WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/builds/head-ON-empty/traffic-gen/tests/traffic_gen-test_UBSAN [==[--gtest_filter=TrafficGenTest.SimpleVarValueInExpectedSet]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[TrafficGenTest.SimpleVarValueInExpectedSet]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[$WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/head/traffic-gen/tests/traffic_gen_test.cpp:117]==]
    WORKING_DIRECTORY [==[$WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/builds/head-ON-empty/traffic-gen/tests]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[TrafficGenTest.DeterministicWithSameSeed]=]  $WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/builds/head-ON-empty/traffic-gen/tests/traffic_gen-test_UBSAN [==[--gtest_filter=TrafficGenTest.DeterministicWithSameSeed]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[TrafficGenTest.DeterministicWithSameSeed]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[$WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/head/traffic-gen/tests/traffic_gen_test.cpp:141]==]
    WORKING_DIRECTORY [==[$WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/builds/head-ON-empty/traffic-gen/tests]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[TrafficGenTest.DifferentSeedsDifferentPackets]=]  $WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/builds/head-ON-empty/traffic-gen/tests/traffic_gen-test_UBSAN [==[--gtest_filter=TrafficGenTest.DifferentSeedsDifferentPackets]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[TrafficGenTest.DifferentSeedsDifferentPackets]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[$WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/head/traffic-gen/tests/traffic_gen_test.cpp:157]==]
    WORKING_DIRECTORY [==[$WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/builds/head-ON-empty/traffic-gen/tests]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[TrafficGenTest.OpenClose]=]  $WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/builds/head-ON-empty/traffic-gen/tests/traffic_gen-test_UBSAN [==[--gtest_filter=TrafficGenTest.OpenClose]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[TrafficGenTest.OpenClose]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[$WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/head/traffic-gen/tests/traffic_gen_test.cpp:182]==]
    WORKING_DIRECTORY [==[$WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/builds/head-ON-empty/traffic-gen/tests]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[TrafficGenTest.SendNamedInterface]=]  $WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/builds/head-ON-empty/traffic-gen/tests/traffic_gen-test_UBSAN [==[--gtest_filter=TrafficGenTest.SendNamedInterface]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[TrafficGenTest.SendNamedInterface]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[$WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/head/traffic-gen/tests/traffic_gen_test.cpp:195]==]
    WORKING_DIRECTORY [==[$WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/builds/head-ON-empty/traffic-gen/tests]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[TrafficGenTest.SendUnknownInterfaceReturnsError]=]  $WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/builds/head-ON-empty/traffic-gen/tests/traffic_gen-test_UBSAN [==[--gtest_filter=TrafficGenTest.SendUnknownInterfaceReturnsError]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[TrafficGenTest.SendUnknownInterfaceReturnsError]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[$WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/head/traffic-gen/tests/traffic_gen_test.cpp:212]==]
    WORKING_DIRECTORY [==[$WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/builds/head-ON-empty/traffic-gen/tests]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[TrafficGenTest.SendAllSendsOnePacketPerInterface]=]  $WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/builds/head-ON-empty/traffic-gen/tests/traffic_gen-test_UBSAN [==[--gtest_filter=TrafficGenTest.SendAllSendsOnePacketPerInterface]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[TrafficGenTest.SendAllSendsOnePacketPerInterface]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[$WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/head/traffic-gen/tests/traffic_gen_test.cpp:223]==]
    WORKING_DIRECTORY [==[$WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/builds/head-ON-empty/traffic-gen/tests]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[TrafficGenTest.SendFilteredInDirection]=]  $WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/builds/head-ON-empty/traffic-gen/tests/traffic_gen-test_UBSAN [==[--gtest_filter=TrafficGenTest.SendFilteredInDirection]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[TrafficGenTest.SendFilteredInDirection]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[$WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/head/traffic-gen/tests/traffic_gen_test.cpp:238]==]
    WORKING_DIRECTORY [==[$WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/builds/head-ON-empty/traffic-gen/tests]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[TrafficGenTest.SendFilteredOutDirectionProducesNone]=]  $WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/builds/head-ON-empty/traffic-gen/tests/traffic_gen-test_UBSAN [==[--gtest_filter=TrafficGenTest.SendFilteredOutDirectionProducesNone]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[TrafficGenTest.SendFilteredOutDirectionProducesNone]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[$WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/head/traffic-gen/tests/traffic_gen_test.cpp:253]==]
    WORKING_DIRECTORY [==[$WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/builds/head-ON-empty/traffic-gen/tests]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[TrafficGenTest.SendLoopProducesNPackets]=]  $WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/builds/head-ON-empty/traffic-gen/tests/traffic_gen-test_UBSAN [==[--gtest_filter=TrafficGenTest.SendLoopProducesNPackets]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[TrafficGenTest.SendLoopProducesNPackets]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[$WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/head/traffic-gen/tests/traffic_gen_test.cpp:268]==]
    WORKING_DIRECTORY [==[$WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/builds/head-ON-empty/traffic-gen/tests]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[TrafficGenTest.SendWithoutOpenReturnsNotOpen]=]  $WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/builds/head-ON-empty/traffic-gen/tests/traffic_gen-test_UBSAN [==[--gtest_filter=TrafficGenTest.SendWithoutOpenReturnsNotOpen]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[TrafficGenTest.SendWithoutOpenReturnsNotOpen]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[$WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/head/traffic-gen/tests/traffic_gen_test.cpp:285]==]
    WORKING_DIRECTORY [==[$WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/builds/head-ON-empty/traffic-gen/tests]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[VerilatorSenderBeat.FullBeat]=]  $WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/builds/head-ON-empty/traffic-gen/tests/traffic_gen-test_UBSAN [==[--gtest_filter=VerilatorSenderBeat.FullBeat]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[VerilatorSenderBeat.FullBeat]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[$WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/head/traffic-gen/tests/traffic_gen_test.cpp:300]==]
    WORKING_DIRECTORY [==[$WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/builds/head-ON-empty/traffic-gen/tests]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[VerilatorSenderBeat.PartialLastBeat]=]  $WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/builds/head-ON-empty/traffic-gen/tests/traffic_gen-test_UBSAN [==[--gtest_filter=VerilatorSenderBeat.PartialLastBeat]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[VerilatorSenderBeat.PartialLastBeat]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[$WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/head/traffic-gen/tests/traffic_gen_test.cpp:326]==]
    WORKING_DIRECTORY [==[$WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/builds/head-ON-empty/traffic-gen/tests]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[WideField.Emits512BitFieldWithoutUB]=]  $WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/builds/head-ON-empty/traffic-gen/tests/traffic_gen-test_UBSAN [==[--gtest_filter=WideField.Emits512BitFieldWithoutUB]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[WideField.Emits512BitFieldWithoutUB]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[$WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/head/traffic-gen/tests/traffic_gen_test.cpp:341]==]
    WORKING_DIRECTORY [==[$WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/builds/head-ON-empty/traffic-gen/tests]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
set(traffic_gen-test_UBSAN_TESTS [==[TrafficGenTest.EmptyInterfaceProducesEmptyPacket]==] [==[TrafficGenTest.SimpleVarProducesOneByte]==] [==[TrafficGenTest.SimpleVarValueInExpectedSet]==] [==[TrafficGenTest.DeterministicWithSameSeed]==] [==[TrafficGenTest.DifferentSeedsDifferentPackets]==] [==[TrafficGenTest.OpenClose]==] [==[TrafficGenTest.SendNamedInterface]==] [==[TrafficGenTest.SendUnknownInterfaceReturnsError]==] [==[TrafficGenTest.SendAllSendsOnePacketPerInterface]==] [==[TrafficGenTest.SendFilteredInDirection]==] [==[TrafficGenTest.SendFilteredOutDirectionProducesNone]==] [==[TrafficGenTest.SendLoopProducesNPackets]==] [==[TrafficGenTest.SendWithoutOpenReturnsNotOpen]==] [==[VerilatorSenderBeat.FullBeat]==] [==[VerilatorSenderBeat.PartialLastBeat]==] [==[WideField.Emits512BitFieldWithoutUB]==])
