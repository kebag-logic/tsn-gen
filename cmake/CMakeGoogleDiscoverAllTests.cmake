macro(gtest_discover_test_wtestlibs EXEC)

	gtest_discover_tests(${EXEC}_UBSAN)
	gtest_discover_tests(${EXEC}_ASAN)
	gtest_discover_tests(${EXEC})

endmacro(gtest_discover_test_wtestlibs)
