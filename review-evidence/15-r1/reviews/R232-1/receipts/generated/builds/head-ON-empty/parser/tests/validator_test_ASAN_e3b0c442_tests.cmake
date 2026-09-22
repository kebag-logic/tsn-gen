add_test([=[Validator.ConformingFilePasses]=]  $WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/builds/head-ON-empty/parser/tests/validator-test_ASAN [==[--gtest_filter=Validator.ConformingFilePasses]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[Validator.ConformingFilePasses]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[$WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/head/parser/tests/validator_test.cpp:40]==]
    WORKING_DIRECTORY [==[$WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/builds/head-ON-empty/parser/tests]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[Validator.ForeignTopLevelKeyRejected]=]  $WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/builds/head-ON-empty/parser/tests/validator-test_ASAN [==[--gtest_filter=Validator.ForeignTopLevelKeyRejected]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[Validator.ForeignTopLevelKeyRejected]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[$WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/head/parser/tests/validator_test.cpp:45]==]
    WORKING_DIRECTORY [==[$WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/builds/head-ON-empty/parser/tests]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[Validator.MissingServiceRejected]=]  $WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/builds/head-ON-empty/parser/tests/validator-test_ASAN [==[--gtest_filter=Validator.MissingServiceRejected]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[Validator.MissingServiceRejected]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[$WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/head/parser/tests/validator_test.cpp:51]==]
    WORKING_DIRECTORY [==[$WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/builds/head-ON-empty/parser/tests]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[Validator.OversizeFieldRejected]=]  $WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/builds/head-ON-empty/parser/tests/validator-test_ASAN [==[--gtest_filter=Validator.OversizeFieldRejected]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[Validator.OversizeFieldRejected]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[$WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/head/parser/tests/validator_test.cpp:57]==]
    WORKING_DIRECTORY [==[$WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/builds/head-ON-empty/parser/tests]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[Validator.BadDirectionRejected]=]  $WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/builds/head-ON-empty/parser/tests/validator-test_ASAN [==[--gtest_filter=Validator.BadDirectionRejected]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[Validator.BadDirectionRejected]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[$WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/head/parser/tests/validator_test.cpp:63]==]
    WORKING_DIRECTORY [==[$WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/builds/head-ON-empty/parser/tests]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[Validator.ServiceWithoutVarsOrEntitiesRejected]=]  $WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/builds/head-ON-empty/parser/tests/validator-test_ASAN [==[--gtest_filter=Validator.ServiceWithoutVarsOrEntitiesRejected]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[Validator.ServiceWithoutVarsOrEntitiesRejected]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[$WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/head/parser/tests/validator_test.cpp:69]==]
    WORKING_DIRECTORY [==[$WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/builds/head-ON-empty/parser/tests]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[Validator.FindingsCarryLineNumbers]=]  $WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/builds/head-ON-empty/parser/tests/validator-test_ASAN [==[--gtest_filter=Validator.FindingsCarryLineNumbers]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[Validator.FindingsCarryLineNumbers]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[$WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/head/parser/tests/validator_test.cpp:76]==]
    WORKING_DIRECTORY [==[$WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/builds/head-ON-empty/parser/tests]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[Validator.MissingDirectoryReported]=]  $WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/builds/head-ON-empty/parser/tests/validator-test_ASAN [==[--gtest_filter=Validator.MissingDirectoryReported]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[Validator.MissingDirectoryReported]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[$WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/head/parser/tests/validator_test.cpp:88]==]
    WORKING_DIRECTORY [==[$WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/builds/head-ON-empty/parser/tests]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
set(validator-test_ASAN_TESTS [==[Validator.ConformingFilePasses]==] [==[Validator.ForeignTopLevelKeyRejected]==] [==[Validator.MissingServiceRejected]==] [==[Validator.OversizeFieldRejected]==] [==[Validator.BadDirectionRejected]==] [==[Validator.ServiceWithoutVarsOrEntitiesRejected]==] [==[Validator.FindingsCarryLineNumbers]==] [==[Validator.MissingDirectoryReported]==])
