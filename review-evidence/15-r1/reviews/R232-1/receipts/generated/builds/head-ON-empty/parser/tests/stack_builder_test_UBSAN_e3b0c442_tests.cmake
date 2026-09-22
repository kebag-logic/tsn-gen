add_test([=[StackBuilder.BuildsAllPassthroughStack]=]  $WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/builds/head-ON-empty/parser/tests/stack_builder-test_UBSAN [==[--gtest_filter=StackBuilder.BuildsAllPassthroughStack]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[StackBuilder.BuildsAllPassthroughStack]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[$WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/head/parser/tests/stack_builder_test.cpp:58]==]
    WORKING_DIRECTORY [==[$WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/builds/head-ON-empty/parser/tests]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[StackBuilder.ResolvesRegisteredLogic]=]  $WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/builds/head-ON-empty/parser/tests/stack_builder-test_UBSAN [==[--gtest_filter=StackBuilder.ResolvesRegisteredLogic]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[StackBuilder.ResolvesRegisteredLogic]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[$WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/head/parser/tests/stack_builder_test.cpp:81]==]
    WORKING_DIRECTORY [==[$WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/builds/head-ON-empty/parser/tests]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[StackBuilder.AdjacencyWiredBottomUp]=]  $WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/builds/head-ON-empty/parser/tests/stack_builder-test_UBSAN [==[--gtest_filter=StackBuilder.AdjacencyWiredBottomUp]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[StackBuilder.AdjacencyWiredBottomUp]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[$WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/head/parser/tests/stack_builder_test.cpp:109]==]
    WORKING_DIRECTORY [==[$WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/builds/head-ON-empty/parser/tests]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[StackBuilder.UnknownLogicFailsBuild]=]  $WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/builds/head-ON-empty/parser/tests/stack_builder-test_UBSAN [==[--gtest_filter=StackBuilder.UnknownLogicFailsBuild]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[StackBuilder.UnknownLogicFailsBuild]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[$WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/head/parser/tests/stack_builder_test.cpp:127]==]
    WORKING_DIRECTORY [==[$WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/builds/head-ON-empty/parser/tests]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[StackBuilder.UnknownServiceFailsBuild]=]  $WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/builds/head-ON-empty/parser/tests/stack_builder-test_UBSAN [==[--gtest_filter=StackBuilder.UnknownServiceFailsBuild]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[StackBuilder.UnknownServiceFailsBuild]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[$WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/head/parser/tests/stack_builder_test.cpp:140]==]
    WORKING_DIRECTORY [==[$WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/builds/head-ON-empty/parser/tests]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[StackBuilder.BypassLogicForcesPassthrough]=]  $WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/builds/head-ON-empty/parser/tests/stack_builder-test_UBSAN [==[--gtest_filter=StackBuilder.BypassLogicForcesPassthrough]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[StackBuilder.BypassLogicForcesPassthrough]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[$WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/head/parser/tests/stack_builder_test.cpp:154]==]
    WORKING_DIRECTORY [==[$WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/builds/head-ON-empty/parser/tests]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[StackBuilder.BypassLogicSkipsRegistryLookup]=]  $WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/builds/head-ON-empty/parser/tests/stack_builder-test_UBSAN [==[--gtest_filter=StackBuilder.BypassLogicSkipsRegistryLookup]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[StackBuilder.BypassLogicSkipsRegistryLookup]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[$WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/head/parser/tests/stack_builder_test.cpp:183]==]
    WORKING_DIRECTORY [==[$WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/builds/head-ON-empty/parser/tests]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[StackBuilder.InvalidPathFailsBuild]=]  $WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/builds/head-ON-empty/parser/tests/stack_builder-test_UBSAN [==[--gtest_filter=StackBuilder.InvalidPathFailsBuild]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[StackBuilder.InvalidPathFailsBuild]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[$WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/head/parser/tests/stack_builder_test.cpp:204]==]
    WORKING_DIRECTORY [==[$WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/builds/head-ON-empty/parser/tests]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
set(stack_builder-test_UBSAN_TESTS [==[StackBuilder.BuildsAllPassthroughStack]==] [==[StackBuilder.ResolvesRegisteredLogic]==] [==[StackBuilder.AdjacencyWiredBottomUp]==] [==[StackBuilder.UnknownLogicFailsBuild]==] [==[StackBuilder.UnknownServiceFailsBuild]==] [==[StackBuilder.BypassLogicForcesPassthrough]==] [==[StackBuilder.BypassLogicSkipsRegistryLookup]==] [==[StackBuilder.InvalidPathFailsBuild]==])
