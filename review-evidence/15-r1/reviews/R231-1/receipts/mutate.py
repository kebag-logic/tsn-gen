#!/usr/bin/env python3
"""mutate.py <tree> <mutation>: apply one named review mutation to a
disposable candidate copy. Each mutation asserts its anchor text exists
exactly once so a silent no-op is impossible."""
import sys, subprocess
tree, m = sys.argv[1], sys.argv[2]
H = f'{tree}/cmake/CMakeGenTestingLibraries.cmake'
L = f'{tree}/cmake/CMakeLinkToTestlibs.cmake'
T = f'{tree}/tests/sanitizer/CMakeLists.txt'
def sub(path, old, new, count=1):
    s = open(path).read()
    assert s.count(old) == count, (path, old, s.count(old))
    open(path, 'w').write(s.replace(old, new))
ASAN_PUB = "\t\t$<$<CONFIG:RelWithDebInfo>:-Os -g3 ${COMP_COVERAGE_FLAGS}>\n\t\t${COMP_ASAN_FLAGS}\n\t)\n"
UBSAN_PUB = "\t\t$<$<CONFIG:RelWithDebInfo>:-Os -g3 ${COMP_COVERAGE_FLAGS}>\n\t\t${COMP_UBSAN_FLAGS}\n\t)\n"
def visibility(vis):
    sub(H, ASAN_PUB, "\t\t$<$<CONFIG:RelWithDebInfo>:-Os -g3 ${COMP_COVERAGE_FLAGS}>\n\t)\n"
        f"\ttarget_compile_options(${{BASENAME}}_asan {vis} ${{COMP_ASAN_FLAGS}})\n")
    sub(H, UBSAN_PUB, "\t\t$<$<CONFIG:RelWithDebInfo>:-Os -g3 ${COMP_COVERAGE_FLAGS}>\n\t)\n"
        f"\ttarget_compile_options(${{BASENAME}}_ubsan {vis} ${{COMP_UBSAN_FLAGS}})\n")
if m == 'M1-base-helper':
    open(H, 'wb').write(subprocess.run(['git', '-C', '$VALIDATION_STORAGE/reviews/r231-18-r1', 'show',
        'deca300c9eb4863fa13383b45ba6cb6fd0828671:cmake/CMakeGenTestingLibraries.cmake'],
        check=True, capture_output=True).stdout)
elif m == 'M2-private-library-only':
    visibility('PRIVATE')
elif m == 'M3-interface-consumer-only':
    visibility('INTERFACE')
elif m == 'M4-no-ubsan-halt':
    sub(T, '\tPROPERTIES ENVIRONMENT "UBSAN_OPTIONS=halt_on_error=1"\n', '\tPROPERTIES LABELS "r231-no-halt"\n')
elif m == 'M5-no-link-runtime':
    sub(L, '\t\t${COMP_UBSAN_FLAGS}\n\t\t${INSTRLIB}_ubsan\n', '\t\t${INSTRLIB}_ubsan\n')
    sub(L, '\t\t${COMP_ASAN_FLAGS}\n\t\t${INSTRLIB}_asan\n', '\t\t${INSTRLIB}_asan\n')
elif m == 'M6a-asan-only-reverted':
    sub(H, ASAN_PUB, "\t\t$<$<CONFIG:RelWithDebInfo>:-Os -g3 ${COMP_COVERAGE_FLAGS}>\n\t)\n")
elif m == 'M6b-ubsan-only-reverted':
    sub(H, UBSAN_PUB, "\t\t$<$<CONFIG:RelWithDebInfo>:-Os -g3 ${COMP_COVERAGE_FLAGS}>\n\t)\n")
elif m == 'M7-faults-under-clean-policy':
    s = open(T).read()
    s += '''
# R231 mutation M7: faulting modes judged by the clean controls' policy.
foreach(_case sanitizer_probe-test_ASAN:library-heap-overflow
		sanitizer_probe-test_ASAN:consumer-heap-overflow
		sanitizer_probe-test_UBSAN:library-signed-overflow
		sanitizer_probe-test_UBSAN:consumer-signed-overflow)
	string(REPLACE ":" ";" _pm "${_case}")
	list(GET _pm 0 _p)
	list(GET _pm 1 _m)
	add_test(NAME r231-${_p}.${_m}.as-clean COMMAND ${_p} ${_m})
	set_tests_properties(r231-${_p}.${_m}.as-clean PROPERTIES
		FAIL_REGULAR_EXPRESSION "Sanitizer|runtime error")
endforeach()
'''
    open(T, 'w').write(s)
elif m == 'M8-plain-library-linked':
    sub(L, '\t\t${COMP_UBSAN_FLAGS}\n\t\t${INSTRLIB}_ubsan\n', '\t\t${COMP_UBSAN_FLAGS}\n\t\t${INSTRLIB}\n')
    sub(L, '\t\t${COMP_ASAN_FLAGS}\n\t\t${INSTRLIB}_asan\n', '\t\t${COMP_ASAN_FLAGS}\n\t\t${INSTRLIB}\n')
else:
    raise SystemExit('unknown mutation ' + m)
print('applied', m)
