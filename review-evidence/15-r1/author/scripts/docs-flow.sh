#!/bin/bash
# Usage: docs-flow.sh <id-prefix> <source-dir> <build-dir> [extra cmake args...]
# The documented unit-test flow (README / docs/low-level/testing-and-ci.md):
#   cmake -B build <args>; cmake --build build -j<N>; ctest via tests/run-tests.sh
# run out of tree with the build capped at 8 jobs. Each step is a separate
# run.sh record, and tests/run-tests.sh is invoked directly.
set -u
E=$WORKSPACE_HOME/milan-fpga-management/2026-09-22/tsngen15-author
id=$1
src=$2
bld=$3
shift 3
rm -rf "$bld"
"$E/scripts/run.sh" "$id-configure" "Documented configure: cmake -S <src> -B <build> $*" \
	env -u CMAKE_BUILD_TYPE cmake -S "$src" -B "$bld" "$@" || exit $?
"$E/scripts/run.sh" "$id-build" "Documented build, capped: cmake --build <build> -j8" \
	cmake --build "$bld" -j8 || exit $?
"$E/scripts/run.sh" "$id-unit" "Unit gate: tests/run-tests.sh <build> invoked directly" \
	"$src/tests/run-tests.sh" "$bld"
