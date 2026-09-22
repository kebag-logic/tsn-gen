add_test([=[LogicRegistry.RegisterAndCreate]=]  $WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/builds/base-ON-empty/parser/tests/logic_registry-test_UBSAN [==[--gtest_filter=LogicRegistry.RegisterAndCreate]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[LogicRegistry.RegisterAndCreate]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[$WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/base/parser/tests/logic_registry_test.cpp:34]==]
    WORKING_DIRECTORY [==[$WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/builds/base-ON-empty/parser/tests]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[LogicRegistry.UnknownNameReturnsNull]=]  $WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/builds/base-ON-empty/parser/tests/logic_registry-test_UBSAN [==[--gtest_filter=LogicRegistry.UnknownNameReturnsNull]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[LogicRegistry.UnknownNameReturnsNull]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[$WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/base/parser/tests/logic_registry_test.cpp:52]==]
    WORKING_DIRECTORY [==[$WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/builds/base-ON-empty/parser/tests]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[LogicRegistry.DuplicateRegistrationRejected]=]  $WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/builds/base-ON-empty/parser/tests/logic_registry-test_UBSAN [==[--gtest_filter=LogicRegistry.DuplicateRegistrationRejected]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[LogicRegistry.DuplicateRegistrationRejected]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[$WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/base/parser/tests/logic_registry_test.cpp:58]==]
    WORKING_DIRECTORY [==[$WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/builds/base-ON-empty/parser/tests]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[LogicRegistry.EmptyNameRejected]=]  $WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/builds/base-ON-empty/parser/tests/logic_registry-test_UBSAN [==[--gtest_filter=LogicRegistry.EmptyNameRejected]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[LogicRegistry.EmptyNameRejected]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[$WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/base/parser/tests/logic_registry_test.cpp:74]==]
    WORKING_DIRECTORY [==[$WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/builds/base-ON-empty/parser/tests]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[LogicRegistry.NullFactoryRejected]=]  $WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/builds/base-ON-empty/parser/tests/logic_registry-test_UBSAN [==[--gtest_filter=LogicRegistry.NullFactoryRejected]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[LogicRegistry.NullFactoryRejected]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[$WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/base/parser/tests/logic_registry_test.cpp:80]==]
    WORKING_DIRECTORY [==[$WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/builds/base-ON-empty/parser/tests]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[PassthroughLogic.IsNoOp]=]  $WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/builds/base-ON-empty/parser/tests/logic_registry-test_UBSAN [==[--gtest_filter=PassthroughLogic.IsNoOp]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[PassthroughLogic.IsNoOp]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[$WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/base/parser/tests/logic_registry_test.cpp:86]==]
    WORKING_DIRECTORY [==[$WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/builds/base-ON-empty/parser/tests]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[LayerContext.AdjacencyAndDefaults]=]  $WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/builds/base-ON-empty/parser/tests/logic_registry-test_UBSAN [==[--gtest_filter=LayerContext.AdjacencyAndDefaults]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[LayerContext.AdjacencyAndDefaults]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[$WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/base/parser/tests/logic_registry_test.cpp:97]==]
    WORKING_DIRECTORY [==[$WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/builds/base-ON-empty/parser/tests]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
set(logic_registry-test_UBSAN_TESTS [==[LogicRegistry.RegisterAndCreate]==] [==[LogicRegistry.UnknownNameReturnsNull]==] [==[LogicRegistry.DuplicateRegistrationRejected]==] [==[LogicRegistry.EmptyNameRejected]==] [==[LogicRegistry.NullFactoryRejected]==] [==[PassthroughLogic.IsNoOp]==] [==[LayerContext.AdjacencyAndDefaults]==])
