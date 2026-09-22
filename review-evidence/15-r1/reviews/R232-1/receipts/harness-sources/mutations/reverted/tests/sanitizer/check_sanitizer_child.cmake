# Runs one sanitizer probe child and checks that its sanitizer stopped it at
# the fault. CTest judges this script, so the child's own exit status and
# output are printed first and the test log keeps them unchanged.
#
#   cmake -DCHILD=<probe> -DMODE=<mode> -DEXPECT_EXIT=<status>
#         -DEXPECT_REPORT=<regex> -P check_sanitizer_child.cmake
cmake_minimum_required(VERSION 3.21)

foreach(_var CHILD MODE EXPECT_EXIT EXPECT_REPORT)
	if(NOT DEFINED ${_var})
		message(FATAL_ERROR "${_var} is not set")
	endif()
endforeach()

# Record the runtime options the child inherits; the test sets any it needs.
foreach(_var ASAN_OPTIONS UBSAN_OPTIONS)
	if(DEFINED ENV{${_var}})
		message(STATUS "${_var}=$ENV{${_var}}")
	else()
		message(STATUS "${_var} is unset")
	endif()
endforeach()

execute_process(
	COMMAND "${CHILD}" "${MODE}"
	RESULT_VARIABLE _exit
	OUTPUT_VARIABLE _out
	ERROR_VARIABLE _err
)

message(STATUS "child: ${CHILD} ${MODE}")
message(STATUS "child exit status: ${_exit}")
message(STATUS "child stdout:\n${_out}")
message(STATUS "child stderr:\n${_err}")

set(_failures "")
if(NOT _exit STREQUAL EXPECT_EXIT)
	string(APPEND _failures "exit status ${_exit}, expected ${EXPECT_EXIT}\n")
endif()
if(NOT _err MATCHES "${EXPECT_REPORT}")
	string(APPEND _failures "no report matching \"${EXPECT_REPORT}\" on stderr\n")
endif()
if(NOT _out MATCHES "${MODE} started")
	string(APPEND _failures "the probe never reached its faulting operation\n")
endif()
if(_out MATCHES "${MODE} completed")
	string(APPEND _failures "the probe continued past its faulting operation\n")
endif()

if(_failures)
	message(FATAL_ERROR "sanitizer diagnostic control failed:\n${_failures}")
endif()
message(STATUS "the sanitizer stopped the probe at its fault, as expected")
