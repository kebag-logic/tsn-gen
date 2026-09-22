#!/bin/bash
# Usage: consumer-controls.sh <label: base|cand>
# Consumer controls for one source tree, each configured, built (8 jobs) and
# run as separate run.sh records, with the generated compile/link/flags files
# captured under receipts/consumers/<label>/<control>/ in the format of
# configure-matrix.sh so compare-configure.py can diff base and candidate:
#   tests-off   top-level tsn-gen with -DENABLE_PARSER_TESTS=OFF, empty type;
#               packet_gen then builds a logic-driven frame
#   embedded-*  scripts/embedded-consumer (add_subdirectory), empty and Debug;
#               the consumer app then builds two frames
set -u
E=$WORKSPACE_HOME/milan-fpga-management/2026-09-22/tsngen15-author
S=$VALIDATION_STORAGE/tmp/a162-tsngen15
lab=$1
src=$S/src/$lab
status=0

capture() {
	local bdir=$1 cdir=$2
	shift 2
	mkdir -p "$cdir"
	printf '%q ' "$@" > "$cdir/configure.argv"
	cp "$bdir/compile_commands.json" "$cdir/"
	(cd "$bdir" && find . -name link.txt -path '*CMakeFiles/*' | sort | while read -r f; do
		mkdir -p "$cdir/link/$(dirname "${f#./}")"; cp "$f" "$cdir/link/${f#./}"; done
	find . -name flags.make -path '*CMakeFiles/*' | sort | while read -r f; do
		mkdir -p "$cdir/flags/$(dirname "${f#./}")"; cp "$f" "$cdir/flags/${f#./}"; done)
	{
		echo "compile commands: $(python3 -c "import json,sys; print(len(json.load(open(sys.argv[1]))))" "$bdir/compile_commands.json")"
		echo "compile commands with -fsanitize: $(grep -c -- '-fsanitize' "$bdir/compile_commands.json")"
		echo "link.txt files with -fsanitize: $(cd "$bdir" && find . -name link.txt -path '*CMakeFiles/*' -exec grep -l -- '-fsanitize' {} + | wc -l)"
		echo "sanitizer variant or probe targets: $(cd "$bdir" && find . -maxdepth 3 -type d \( -name '*_asan.dir' -o -name '*_ubsan.dir' -o -name '*_ASAN.dir' -o -name '*_UBSAN.dir' -o -name 'sanitizer_probe*.dir' \) | wc -l)"
		echo "CTest file present: $(test -f "$bdir/CTestTestfile.cmake" && echo yes || echo no)"
	} > "$cdir/sanitizer-summary.txt"
	cat "$cdir/sanitizer-summary.txt"
}

# tests-off top level
b=$S/build/consumer-$lab-tests-off
rm -rf "$b"
args=(-S "$src" -B "$b" -G "Unix Makefiles" -DENABLE_PARSER_TESTS=OFF -DCMAKE_EXPORT_COMPILE_COMMANDS=ON)
"$E/scripts/run.sh" "080-$lab-tests-off-configure" "Top-level tsn-gen with ENABLE_PARSER_TESTS=OFF, empty build type" \
	env -u CMAKE_BUILD_TYPE cmake "${args[@]}" || exit $?
"$E/scripts/run.sh" "080-$lab-tests-off-build" "Build tests-off tree, capped at 8 jobs" cmake --build "$b" -j8 || status=1
"$E/scripts/run.sh" "080-$lab-tests-off-run" "packet_gen builds one logic-driven AECP ACQUIRE_ENTITY frame" \
	"$b/traffic-gen/packet_gen" --yaml-dir "$src/protocols" --stack-file "$src/stacks/aecp_acquire_entity.yaml" --seed 42 --count 1 || status=1
capture "$b" "$E/receipts/consumers/$lab/tests-off" "${args[@]}"

# embedded consumer, empty and Debug
for cfg in empty Debug; do
	b=$S/build/consumer-$lab-embedded-$cfg
	rm -rf "$b"
	args=(-S "$E/scripts/embedded-consumer" -B "$b" -G "Unix Makefiles" -DTSN_GEN_SOURCE_DIR="$src" -DCMAKE_EXPORT_COMPILE_COMMANDS=ON)
	[ "$cfg" != empty ] && args+=(-DCMAKE_BUILD_TYPE="$cfg")
	"$E/scripts/run.sh" "081-$lab-embedded-$cfg-configure" "Embedded add_subdirectory consumer, build type '$cfg'" \
		env -u CMAKE_BUILD_TYPE cmake "${args[@]}" || exit $?
	"$E/scripts/run.sh" "081-$lab-embedded-$cfg-build" "Build embedded consumer, capped at 8 jobs" cmake --build "$b" -j8 || status=1
	"$E/scripts/run.sh" "081-$lab-embedded-$cfg-run" "Run the embedded consumer app" "$b/app" || status=1
	grep -E '^ENABLE_PARSER_TESTS' "$b/CMakeCache.txt" > "$E/logs/081-$lab-embedded-$cfg-cache.txt"
	capture "$b" "$E/receipts/consumers/$lab/embedded-$cfg" "${args[@]}"
done
exit $status
