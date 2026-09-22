#!/bin/bash
# Usage: cross-compile-control.sh <id> <source-dir>
# Second-compiler control, compile-only: configures <source-dir> with the
# aarch64-linux-gnu GCC cross compiler (empty build type, BUILD_SHARED_LIBS=ON,
# Unix Makefiles), builds the probe and protocol_parser sanitizer variant
# libraries (shared, so they link without a runtime) and compiles the probe
# and ptp_flags test objects of the _ASAN/_UBSAN executables. It then records
# the sanitizer references each object holds (aarch64-linux-gnu-nm -u) and
# attempts one sanitized executable link, which needs a target sanitizer
# runtime this toolchain does not ship.
set -u
E=$WORKSPACE_HOME/milan-fpga-management/2026-09-22/tsngen15-author
S=$VALIDATION_STORAGE/tmp/a162-tsngen15
id=$1
src=$2
bld=$S/build/cross-$id
out=$E/receipts/cross/$id
rm -rf "$bld" "$out"
mkdir -p "$out"
"$E/scripts/run.sh" "$id-configure" "Configure with aarch64-linux-gnu-g++ (cross), empty build type" \
	env -u CMAKE_BUILD_TYPE cmake -S "$src" -B "$bld" -G "Unix Makefiles" \
	-DCMAKE_SYSTEM_NAME=Linux -DCMAKE_SYSTEM_PROCESSOR=aarch64 \
	-DCMAKE_CXX_COMPILER=aarch64-linux-gnu-g++ -DBUILD_SHARED_LIBS=ON \
	-DCMAKE_EXPORT_COMPILE_COMMANDS=ON || exit $?
"$E/scripts/run.sh" "$id-libs" "Build sanitizer variant shared libraries with the cross compiler, 8 jobs" \
	cmake --build "$bld" -j8 --target sanitizer_probe_asan sanitizer_probe_ubsan protocol_parser_asan protocol_parser_ubsan
libs=$?
objs=(
	tests/sanitizer/CMakeFiles/sanitizer_probe-test_ASAN.dir/sanitizer_probe_test.cpp.o
	tests/sanitizer/CMakeFiles/sanitizer_probe-test_UBSAN.dir/sanitizer_probe_test.cpp.o
	traffic-gen/tests/CMakeFiles/ptp_flags-test_ASAN.dir/ptp_flags_test.cpp.o
	traffic-gen/tests/CMakeFiles/ptp_flags-test_UBSAN.dir/ptp_flags_test.cpp.o
)
for o in "${objs[@]}"; do
	d=$(dirname "$(dirname "$(dirname "$o")")")
	t=$(basename "$(dirname "$o")")
	"$E/scripts/run.sh" "$id-obj-${t%.dir}" "Compile $o only, with the cross compiler" \
		make -C "$bld" -f "$d/CMakeFiles/$t/build.make" "$o"
done
"$E/scripts/run.sh" "$id-exe-link" "Attempt the sanitizer_probe-test_ASAN link with the cross compiler" \
	cmake --build "$bld" -j8 --target sanitizer_probe-test_ASAN
link=$?
python3 - "$bld" "$out/objects.json" <<'PY'
import json, os, re, subprocess, sys
bld, out = sys.argv[1:3]
cc = {e["output"]: e["command"] for e in json.load(open(os.path.join(bld, "compile_commands.json")))}
res = {}
for dp, _, fns in os.walk(bld):
    for fn in fns:
        if not fn.endswith(".o") or "sanitizer_probe" not in dp and "protocol_parser_" not in dp and "ptp_flags" not in dp:
            continue
        p = os.path.join(dp, fn)
        rel = os.path.relpath(p, bld)
        syms = subprocess.run(["aarch64-linux-gnu-nm", "-u", p], capture_output=True, text=True).stdout.split()
        cmd = cc.get(rel, "")
        res[rel] = {
            "compile_flags": sorted({t for t in cmd.split() if t.startswith("-fsanitize")}),
            "asan_refs": sum(1 for s in syms if s.startswith("__asan_")),
            "ubsan_refs": sum(1 for s in syms if s.startswith("__ubsan_handle_")),
            "file": subprocess.run(["file", "-b", p], capture_output=True, text=True).stdout.strip()[:40],
        }
json.dump(res, open(out, "w"), indent=1, sort_keys=True)
for k, v in sorted(res.items()):
    print(k, v["compile_flags"], "asan", v["asan_refs"], "ubsan", v["ubsan_refs"])
PY
echo "variant libs build exit $libs; sanitized executable link exit $link"
grep -m2 -E "cannot find -l(a|ub)san" "$E/logs/$id-exe-link.log"
exit 0
