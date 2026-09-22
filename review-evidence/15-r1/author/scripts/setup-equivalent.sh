#!/bin/bash
# Usage: setup-equivalent.sh <id-prefix> <source-dir> <build-dir> [extra cmake args...]
# The configure, build, prepare and unit-test steps of setup.sh, run out of
# tree with the build capped at 8 jobs. Each step is its own run.sh record so
# its exit status is kept; tests/run-tests.sh is invoked directly instead of
# inferring its result from setup.sh's final `cd`.
set -u
E=$WORKSPACE_HOME/milan-fpga-management/2026-09-22/tsngen15-author
id=$1
src=$2
bld=$3
shift 3
rm -rf "$bld"
mkdir -p "$bld"
cd "$bld" || exit 255
"$E/scripts/run.sh" "$id-configure" "setup.sh configure step: cmake <src> -DBUILD_SHARED_LIBS=ON $*" \
	env -u CMAKE_BUILD_TYPE cmake "$src" -DBUILD_SHARED_LIBS=ON "$@" || exit $?
"$E/scripts/run.sh" "$id-build" "setup.sh build step, capped: make -j8" \
	make -j8 || exit $?
"$E/scripts/run.sh" "$id-prep" "setup.sh prepare step: parser/tests/prep-tests.sh" \
	"$src/parser/tests/prep-tests.sh"
prep=$?
"$E/scripts/run.sh" "$id-unit" "Unit gate: tests/run-tests.sh <build> invoked directly" \
	"$src/tests/run-tests.sh" "$bld"
unit=$?
echo "prep exit $prep, unit exit $unit"
[ $prep -ne 0 ] && exit $prep
exit $unit
