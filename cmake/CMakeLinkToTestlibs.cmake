macro(target_link_testlibs TARGETEX INSTRLIB EXESRCS OTHER_LIBS)
	# This way, multiple libs can be linked together
	set(_LIBS ${OTHER_LIBS} ${ARGN})


	# Link the ubsan
	add_executable(
		${TARGETEX}_UBSAN
		${EXESRCS}
	)
	target_link_libraries(
		${TARGETEX}_UBSAN
		${COMP_UBSAN_FLAGS}
		${INSTRLIB}_ubsan
		${_LIBS}

	)

	# Then the ASAN
	add_executable(
		${TARGETEX}_ASAN
		${EXESRCS}
	)
	target_link_libraries(
		${TARGETEX}_ASAN
		${COMP_ASAN_FLAGS}
		${INSTRLIB}_asan
		${_LIBS}
	)

	# Then the NORMAL library
	add_executable(
		${TARGETEX}
		${EXESRCS}
	)

	target_link_libraries(
		${TARGETEX}
		${INSTRLIB}
		${_LIBS}
	)

endmacro(target_link_testlibs)
