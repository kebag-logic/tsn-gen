#!/usr/bin/env python3
"""instrumentation.py <build_dir> <out.json>: per object, record whether it
references ASan (__asan_) / UBSan (__ubsan_handle_) entry points (nm -u:
compiler instrumentation); per ELF artifact, record NEEDED sanitizer runtimes
and undefined sanitizer dynamic symbols (runtime linkage)."""
import sys, os, re, json, subprocess, glob
B, OUT = sys.argv[1], sys.argv[2]
def target_of(p):
    m = re.search(r'CMakeFiles/([^/]+)\.dir/', p); return m.group(1) if m else None
objs = {}
for o in glob.glob(f'{B}/**/*.o', recursive=True):
    if '/CMakeFiles/' not in o or 'CompilerId' in o or 'CMakeScratch' in o: continue
    und = subprocess.run(['nm', '-u', o], capture_output=True, text=True).stdout
    objs[os.path.relpath(o, B)] = {'target': target_of(o),
        'asan_refs': len(re.findall(r'\b__asan_\w+', und)),
        'ubsan_refs': len(re.findall(r'\b__ubsan_handle_\w+', und))}
arts = {}
for f in glob.glob(f'{B}/**/*', recursive=True):
    if not os.path.isfile(f) or os.path.islink(f) or f.endswith('.o'): continue
    with open(f, 'rb') as fh:
        if fh.read(4) != b'\x7fELF': continue
    if '/CMakeFiles/' in f: continue
    d = subprocess.run(['readelf', '-d', f], capture_output=True, text=True).stdout
    needed = re.findall(r'\(NEEDED\)\s+Shared library: \[([^\]]+)\]', d)
    dyn = subprocess.run(['nm', '-D', '-u', f], capture_output=True, text=True).stdout
    arts[os.path.relpath(f, B)] = {
        'sanitizer_needed': [n for n in needed if re.match(r'lib(asan|ubsan|lsan|tsan)\.so', n)],
        'undef_asan': len(re.findall(r'\b__asan_\w+', dyn)),
        'undef_ubsan': len(re.findall(r'\b__ubsan_handle_\w+', dyn))}
def kind(t):
    if t is None: return 'other'
    if t.endswith(('_asan', '_ASAN')): return 'asan'
    if t.endswith(('_ubsan', '_UBSAN')): return 'ubsan'
    return 'plain'
summ = {'objects': len(objs), 'by_kind': {}}
for k in ('asan', 'ubsan', 'plain', 'other'):
    sel = {p: v for p, v in objs.items() if kind(v['target']) == k}
    summ['by_kind'][k] = {'objects': len(sel),
        'with_asan_refs': sum(v['asan_refs'] > 0 for v in sel.values()),
        'with_ubsan_refs': sum(v['ubsan_refs'] > 0 for v in sel.values()),
        'without_expected_refs': sorted(p for p, v in sel.items()
            if (k == 'asan' and not v['asan_refs']) or (k == 'ubsan' and not v['ubsan_refs'])),
        'with_any_refs': sorted(p for p, v in sel.items() if v['asan_refs'] or v['ubsan_refs']) if k in ('plain', 'other') else None}
summ['artifacts_with_sanitizer_needed'] = {p: v['sanitizer_needed'] for p, v in sorted(arts.items()) if v['sanitizer_needed']}
summ['artifacts_with_undef_sanitizer_syms_but_no_needed'] = sorted(p for p, v in arts.items() if (v['undef_asan'] or v['undef_ubsan']) and not v['sanitizer_needed'])
json.dump({'summary': summ, 'objects': objs, 'artifacts': arts}, open(OUT, 'w'), indent=1)
print(json.dumps(summ, indent=1))
