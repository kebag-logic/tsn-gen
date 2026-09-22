#!/bin/bash
# Usage: mutation.sh <id> <name> <build-type|empty> [<patch>]
# Copies the exported candidate tree, applies <patch> (none for the unmutated
# control), configures it as setup.sh does (Unix Makefiles,
# BUILD_SHARED_LIBS=ON) plus the given build type and compile-command export,
# builds only the probe targets (8 jobs) and runs the probe's CTest cases.
# Each step is a separate run.sh record; the probe objects' instrumentation
# and the CTest log are kept under receipts/mutations/<id>/.
set -u
E=$WORKSPACE_HOME/milan-fpga-management/2026-09-22/tsngen15-author
S=$VALIDATION_STORAGE/tmp/a162-tsngen15
id=$1
name=$2
cfg=$3
patch=${4:-}
src=$S/src/mut-$id
bld=$S/build/mut-$id
out=$E/receipts/mutations/$id
rm -rf "$src" "$bld" "$out"
mkdir -p "$out"
cp -a "$S/src/cand" "$src"
if [ -n "$patch" ]; then
	"$E/scripts/run.sh" "$id-apply" "Apply mutation $name to a copy of the candidate" \
		patch -p1 -d "$src" -i "$patch" || exit $?
fi
args=(-S "$src" -B "$bld" -G "Unix Makefiles" -DBUILD_SHARED_LIBS=ON -DCMAKE_EXPORT_COMPILE_COMMANDS=ON)
[ "$cfg" != empty ] && args+=(-DCMAKE_BUILD_TYPE="$cfg")
"$E/scripts/run.sh" "$id-configure" "Configure mutation $name, build type '$cfg'" \
	env -u CMAKE_BUILD_TYPE cmake "${args[@]}" || exit $?
"$E/scripts/run.sh" "$id-build" "Build only the probe targets of mutation $name, capped at 8 jobs" \
	cmake --build "$bld" -j8 --target sanitizer_probe-test sanitizer_probe-test_ASAN sanitizer_probe-test_UBSAN
build=$?
grep -E 'sanitizer_probe' "$bld/compile_commands.json" | grep '"command"' > "$out/probe-compile-commands.txt"
for t in sanitizer_probe_asan sanitizer_probe_ubsan sanitizer_probe-test_ASAN sanitizer_probe-test_UBSAN; do
	cp "$bld/tests/sanitizer/CMakeFiles/$t.dir/link.txt" "$out/$t.link.txt" 2>/dev/null
done
python3 "$E/scripts/instrumentation.py" "$bld" "$out/instrumentation.json" > "$out/instrumentation-summary.json" 2>&1
if [ $build -ne 0 ]; then
	echo "build exit $build; probe CTest not run"
	exit $build
fi
"$E/scripts/run.sh" "$id-ctest" "Run the probe CTest cases of mutation $name" \
	ctest --test-dir "$bld" -R '^sanitizer_probe' --output-on-failure
rc=$?
cp "$bld/Testing/Temporary/LastTest.log" "$out/LastTest.log"
python3 "$E/scripts/scan-test-log.py" "$bld" "$out/scan.json" > /dev/null
exit $rc
