#!/bin/bash
# run_mutation.sh <mutation|M0-none> <build-type|empty>: disposable candidate
# copy -> optional mutation -> configure (BUILD_SHARED_LIBS=ON) -> build the
# probe targets only (8 jobs) -> run the probe CTest cases verbosely.
R=$WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r231/receipts
S=$VALIDATION_STORAGE/reviews/r231-18-r1-scratch
m=$1; bt=$2; id=$m-$bt
src=$S/src/mut/$id; bld=$S/build/mut/$id; out=$R/mutations/$id
rm -rf "$src" "$bld"; mkdir -p "$S/src/mut" "$out"
cp -a $S/src/cand-pristine "$src"
if [ "$m" != M0-none ]; then python3 $R/mutate.py "$src" "$m" > "$out/apply.txt" || exit 90; fi
(cd $S/src && diff -ru cand-pristine "mut/$id" > "$out/mutation.diff"); echo "diff exit $?" >> "$out/apply.txt"
args=(cmake -S "$src" -B "$bld" -DBUILD_SHARED_LIBS=ON -DCMAKE_EXPORT_COMPILE_COMMANDS=ON)
[ "$bt" != empty ] && args+=(-DCMAKE_BUILD_TYPE=$bt)
$R/rr.py 3x-$id-configure $S -- "${args[@]}" > /dev/null
$R/rr.py 3x-$id-build $S -- cmake --build "$bld" -j8 --target sanitizer_probe-test sanitizer_probe-test_ASAN sanitizer_probe-test_UBSAN > /dev/null
echo "build exit $?" > "$out/build-exit.txt"
$R/rr.py 3x-$id-ctest $S -- ctest --test-dir "$bld" -R '^(r231-)?sanitizer_probe' -V > /dev/null
echo "ctest exit $?" > "$out/ctest-exit.txt"
cp "$bld/Testing/Temporary/LastTest.log" "$out/LastTest.log" 2>/dev/null
python3 - "$bld" "$out" <<'PY'
import sys, json, subprocess, re, os, glob
bld, out = sys.argv[1:3]
res = {'objects': {}, 'exes': {}}
for o in sorted(glob.glob(f'{bld}/tests/sanitizer/CMakeFiles/*/*.o')):
    u = subprocess.run(['nm', '-u', o], capture_output=True, text=True).stdout
    res['objects'][os.path.relpath(o, bld)] = {'asan_refs': len(re.findall(r'__asan_\w+', u)), 'ubsan_refs': len(re.findall(r'__ubsan_handle_\w+', u))}
for e in ('sanitizer_probe-test', 'sanitizer_probe-test_ASAN', 'sanitizer_probe-test_UBSAN'):
    p = f'{bld}/tests/sanitizer/{e}'
    if os.path.exists(p):
        d = subprocess.run(['readelf', '-d', p], capture_output=True, text=True).stdout
        res['exes'][e] = [n for n in re.findall(r'\[(lib[^\]]+)\]', d) if re.match(r'lib(asan|ubsan)', n)]
    else:
        res['exes'][e] = 'MISSING'
cc = json.load(open(f'{bld}/compile_commands.json'))
res['probe_compile_flags'] = {os.path.relpath(e['output'] if 'output' in e else e['file'], bld) if False else e['file'].rsplit('/',1)[-1] + ' @ ' + re.search(r'CMakeFiles/([^/]+)\.dir', e['command']).group(1): sorted(set(re.findall(r'-fsanitize=\w+', e['command']))) for e in cc if '/tests/sanitizer/' in e['file']}
json.dump(res, open(f'{out}/instrumentation.json', 'w'), indent=1)
PY
t=$R/logs/3x-$id-ctest.log
python3 - "$t" "$out" <<'PY'
import sys, re
t = open(sys.argv[1], errors='replace').read()
rows = re.findall(r'^\s*\d+/\d+ Test\s+#\d+: (\S+) \.+\s*(\*\*\*Failed|Passed|Not Run|\*\*\*Not Run)', t, re.M)
summ = re.search(r'(\d+% tests passed.*)', t)
with open(sys.argv[2] + '/summary.txt', 'w') as f:
    for n, s in rows: f.write(f'{s:>12}  {n}\n')
    f.write((summ.group(1) if summ else 'no summary') + '\n')
PY
echo "== $id: $(cat $out/build-exit.txt); $(cat $out/ctest-exit.txt)"; cat "$out/summary.txt"
