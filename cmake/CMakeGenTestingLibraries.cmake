macro(generate_test_libs BASENAME INCDIRS SOURCEFILES)

	set(_SOURCEFILES ${SOURCEFILES} ${ARGN})
	if (NOT ${TEST_ENABLED})
		message(WARNING "Test are disable for ${BASENAME}")
		return()
	endif()

	message(STATUS "Generating Multi-library ${COMP_UBSAN_FLAGS}")

	add_library(${BASENAME}_ubsan SHARED
		${_SOURCEFILES}
	)

	add_library(${BASENAME}_asan SHARED
		${_SOURCEFILES}
	)

	target_compile_options(${BASENAME}_asan PUBLIC
		$<$<CONFIG:Debug>:-O0 -g3  ${COMP_COVERAGE_FLAGS} ${COMP_ASAN_FLAGS}>
		$<$<CONFIG:Release>:-O3 -g0 ${COMP_COVERAGE_FLAGS} ${COMP_ASAN_FLAGS}>
		$<$<CONFIG:MinSizeRel>:-Os -g0 ${COMP_COVERAGE_FLAGS} ${COMP_ASAN_FLAGS}>
		$<$<CONFIG:RelWithDebInfo>:-Os -g3 ${COMP_COVERAGE_FLAGS} ${COMP_ASAN_FLAGS}>
	)

	target_include_directories(${BASENAME}_asan PUBLIC
		$<BUILD_INTERFACE:${INCDIRS}>
		$<INSTALL_INTERFACE:include/${BASENAME}>
	)

	target_compile_options(${BASENAME}_ubsan PUBLIC
		$<$<CONFIG:Debug>:-O0 -g3 ${COMP_COVERAGE_FLAGS} ${COMP_UBSAN_FLAGS}>
		$<$<CONFIG:Release>:-O3 -g0 ${COMP_COVERAGE_FLAGS} ${COMP_UBSAN_FLAGS}>
		$<$<CONFIG:MinSizeRel>:-Os -g0 ${COMP_COVERAGE_FLAGS} ${COMP_UBSAN_FLAGS}>
		$<$<CONFIG:RelWithDebInfo>:-Os -g3 ${COMP_COVERAGE_FLAGS} ${COMP_UBSAN_FLAGS}>
	)

	target_include_directories(${BASENAME}_ubsan PUBLIC
		$<BUILD_INTERFACE:${INCDIRS}>
		$<INSTALL_INTERFACE:include/${BASENAME}>
	)

endmacro(generate_test_libs)
