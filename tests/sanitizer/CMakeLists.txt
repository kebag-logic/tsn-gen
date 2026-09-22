# Checks the sanitizer helpers themselves. The fixture library and the probe
# are built by generate_test_libs and target_link_testlibs, as the component
# tests are, so a variant library or a consuming test TU left uninstrumented
# under the current build type fails a diagnostic control below.

##################### MODULES   ####################
include(CMakeGenTestingLibraries)
include(CMakeLinkToTestlibs)

#################### FIXTURE LIBRARY ###############
set(SANITIZER_PROBE_LIB_SRCS
	sanitizer_probe_lib.cpp
)

add_library(sanitizer_probe SHARED
	${SANITIZER_PROBE_LIB_SRCS}
)

target_include_directories(sanitizer_probe PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}
)

generate_test_libs(
	sanitizer_probe
	"${CMAKE_CURRENT_SOURCE_DIR}"
	${SANITIZER_PROBE_LIB_SRCS}
)

#################### PROBE ##########################
# Emits sanitizer_probe-test, _ASAN and _UBSAN; no other library is needed.
target_link_testlibs(
	sanitizer_probe-test
	sanitizer_probe
	sanitizer_probe_test.cpp
	""
)

#################### CONTROLS #######################
# Passing controls: each executable exits 0 from a clean run, with no report.
foreach(_probe sanitizer_probe-test sanitizer_probe-test_ASAN
		sanitizer_probe-test_UBSAN)
	add_test(NAME ${_probe}.clean COMMAND ${_probe} clean)
	set_tests_properties(${_probe}.clean PROPERTIES
		FAIL_REGULAR_EXPRESSION "Sanitizer|runtime error"
	)
endforeach()

# Diagnostic controls: the checker keeps the child's exit status and output,
# and requires status 1, the expected report, and no progress past the fault.
function(add_sanitizer_diagnostic_test PROBE MODE REPORT)
	add_test(NAME ${PROBE}.${MODE}
		COMMAND ${CMAKE_COMMAND}
			-DCHILD=$<TARGET_FILE:${PROBE}>
			-DMODE=${MODE}
			-DEXPECT_EXIT=1
			"-DEXPECT_REPORT=${REPORT}"
			-P ${CMAKE_CURRENT_SOURCE_DIR}/check_sanitizer_child.cmake
	)
endfunction()

add_sanitizer_diagnostic_test(sanitizer_probe-test_ASAN library-heap-overflow
	"ERROR: AddressSanitizer: heap-buffer-overflow")
add_sanitizer_diagnostic_test(sanitizer_probe-test_ASAN consumer-heap-overflow
	"ERROR: AddressSanitizer: heap-buffer-overflow")

# UBSan names the faulting source line; it matches the TU holding the defect.
add_sanitizer_diagnostic_test(sanitizer_probe-test_UBSAN library-signed-overflow
	"sanitizer_probe_lib[.]cpp:[0-9]+:[0-9]+: runtime error: signed integer overflow")
add_sanitizer_diagnostic_test(sanitizer_probe-test_UBSAN consumer-signed-overflow
	"sanitizer_probe_test[.]cpp:[0-9]+:[0-9]+: runtime error: signed integer overflow")

# An ASan report stops the process by default, but UBSan reports and carries
# on. These two controls alone make UBSan fatal; every other test keeps the
# default recovery.
set_tests_properties(
	sanitizer_probe-test_UBSAN.library-signed-overflow
	sanitizer_probe-test_UBSAN.consumer-signed-overflow
	PROPERTIES ENVIRONMENT "UBSAN_OPTIONS=halt_on_error=1"
)
