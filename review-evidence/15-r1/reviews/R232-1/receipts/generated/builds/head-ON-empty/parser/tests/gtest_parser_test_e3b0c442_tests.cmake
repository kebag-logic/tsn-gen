add_test([=[HelloTest.BasicAssertions]=]  $WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/builds/head-ON-empty/parser/tests/gtest_parser-test [==[--gtest_filter=HelloTest.BasicAssertions]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[HelloTest.BasicAssertions]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[$WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/head/parser/tests/gtest.cpp:18]==]
    WORKING_DIRECTORY [==[$WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/builds/head-ON-empty/parser/tests]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
set(gtest_parser-test_TESTS [==[HelloTest.BasicAssertions]==])
