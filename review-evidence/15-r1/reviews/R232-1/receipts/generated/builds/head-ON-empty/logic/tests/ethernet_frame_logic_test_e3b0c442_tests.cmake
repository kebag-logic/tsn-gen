add_test([=[EthernetFrameLogic.RegisteredUnderExpectedName]=]  $WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/builds/head-ON-empty/logic/tests/ethernet_frame_logic-test [==[--gtest_filter=EthernetFrameLogic.RegisteredUnderExpectedName]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[EthernetFrameLogic.RegisteredUnderExpectedName]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[$WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/head/logic/tests/ethernet_frame_logic_test.cpp:38]==]
    WORKING_DIRECTORY [==[$WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/builds/head-ON-empty/logic/tests]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[EthernetFrameLogic.BoundByStackBuilder]=]  $WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/builds/head-ON-empty/logic/tests/ethernet_frame_logic-test [==[--gtest_filter=EthernetFrameLogic.BoundByStackBuilder]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[EthernetFrameLogic.BoundByStackBuilder]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[$WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/head/logic/tests/ethernet_frame_logic_test.cpp:46]==]
    WORKING_DIRECTORY [==[$WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/builds/head-ON-empty/logic/tests]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[EthernetFrameLogic.BypassLogicSwapsInPassthrough]=]  $WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/builds/head-ON-empty/logic/tests/ethernet_frame_logic-test [==[--gtest_filter=EthernetFrameLogic.BypassLogicSwapsInPassthrough]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[EthernetFrameLogic.BypassLogicSwapsInPassthrough]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[$WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/head/logic/tests/ethernet_frame_logic_test.cpp:64]==]
    WORKING_DIRECTORY [==[$WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/builds/head-ON-empty/logic/tests]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[EthernetFrameLogic.EncodeDecodeCountersAdvance]=]  $WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/builds/head-ON-empty/logic/tests/ethernet_frame_logic-test [==[--gtest_filter=EthernetFrameLogic.EncodeDecodeCountersAdvance]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[EthernetFrameLogic.EncodeDecodeCountersAdvance]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[$WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/head/logic/tests/ethernet_frame_logic_test.cpp:85]==]
    WORKING_DIRECTORY [==[$WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/builds/head-ON-empty/logic/tests]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[EthernetFrameLogic.NextLayerDefaults1722]=]  $WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/builds/head-ON-empty/logic/tests/ethernet_frame_logic-test [==[--gtest_filter=EthernetFrameLogic.NextLayerDefaults1722]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[EthernetFrameLogic.NextLayerDefaults1722]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[$WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/head/logic/tests/ethernet_frame_logic_test.cpp:100]==]
    WORKING_DIRECTORY [==[$WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/builds/head-ON-empty/logic/tests]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
set(ethernet_frame_logic-test_TESTS [==[EthernetFrameLogic.RegisteredUnderExpectedName]==] [==[EthernetFrameLogic.BoundByStackBuilder]==] [==[EthernetFrameLogic.BypassLogicSwapsInPassthrough]==] [==[EthernetFrameLogic.EncodeDecodeCountersAdvance]==] [==[EthernetFrameLogic.NextLayerDefaults1722]==])
