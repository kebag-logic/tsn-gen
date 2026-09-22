#!/usr/bin/env python3
"""Record compiler instrumentation and runtime linkage of a built tree.

Usage: instrumentation.py <build-dir> <out-json>

Compiler instrumentation, per object under each CMakeFiles/<target>.dir:
the sanitizer flags on the target's generated CXX_FLAGS (flags.make; no
source here has per-file options), and the sanitizer entry points the
object itself references (`nm -u`): __asan_* for ASan, __ubsan_handle_* for
UBSan.

Runtime linkage, per linked artifact (every CMakeFiles/*/link.txt): the
sanitizer flags on its link command, the sanitizer runtimes it records as
NEEDED (`readelf -d`), and for shared libraries the sanitizer symbols left
undefined for the loading executable to provide (`nm -D -u`).
"""
import json
import os
import re
import shlex
import subprocess
import sys

bld, out_json = sys.argv[1:3]
SAN = {"-fsanitize=address": "address", "-fsanitize=undefined": "undefined"}


def run(*argv):
    return subprocess.run(argv, capture_output=True, text=True, check=True).stdout


def refs(text):
    syms = set(text.split())
    return {
        "address": sorted(s for s in syms if s.startswith("__asan_")),
        "undefined": sorted(s for s in syms if s.startswith("__ubsan_handle_")),
    }


objects = {}
for dp, _, fns in os.walk(bld):
    if "flags.make" not in fns or not dp.endswith(".dir"):
        continue
    cxx = [l for l in open(os.path.join(dp, "flags.make")).read().splitlines()
           if l.startswith("CXX_FLAGS = ")]
    if not cxx:
        continue
    flags = sorted({SAN[t] for t in shlex.split(cxx[0][len("CXX_FLAGS = "):]) if t in SAN})
    for odp, _, ofns in os.walk(dp):
        for fn in ofns:
            if not fn.endswith(".o"):
                continue
            obj = os.path.join(odp, fn)
            r = refs(run("nm", "-u", obj))
            objects[os.path.relpath(obj, bld)] = {
                "compile_flags": flags,
                "observed": sorted(k for k, v in r.items() if v),
                "asan_refs": len(r["address"]), "ubsan_refs": len(r["undefined"]),
                "sample": (r["address"][:3] + r["undefined"][:3]),
            }

artifacts = {}
for dp, _, fns in os.walk(bld):
    if "link.txt" not in fns or "CMakeFiles" not in dp:
        continue
    cmd = open(os.path.join(dp, "link.txt")).read().split("\n")[0]
    toks = shlex.split(cmd)
    if "-o" not in toks:
        continue  # static archive rules
    target_dir = os.path.dirname(os.path.dirname(dp))
    art = os.path.normpath(os.path.join(target_dir, toks[toks.index("-o") + 1]))
    if not os.path.exists(art):
        continue
    dyn = run("readelf", "-d", art)
    needed = re.findall(r"\(NEEDED\)\s+Shared library: \[([^\]]+)\]", dyn)
    rec = {
        "link_flags": sorted({SAN[t] for t in toks if t in SAN}),
        "needed_runtimes": sorted(n for n in needed if re.match(r"lib(a|ub)san\.so", n)),
        "shared": "-shared" in toks,
    }
    if rec["shared"]:
        r = refs(" ".join(l.split()[-1] for l in run("nm", "-D", "-u", art).splitlines() if l.strip()))
        rec["undefined_dynamic_sanitizer_symbols"] = {k: len(v) for k, v in r.items()}
    artifacts[os.path.relpath(art, bld)] = rec

mismatch = {k: v for k, v in objects.items() if v["compile_flags"] != v["observed"]}
summary = {
    "build": bld,
    "objects": len(objects),
    "objects_by_compile_flags": {},
    "objects_by_observed": {},
    "flag_vs_observed_mismatches": mismatch,
    "artifacts_linking_a_runtime": sorted(k for k, v in artifacts.items() if v["needed_runtimes"]),
    "shared_libs_with_undefined_sanitizer_symbols": sorted(
        k for k, v in artifacts.items() if v.get("undefined_dynamic_sanitizer_symbols")
        and any(v["undefined_dynamic_sanitizer_symbols"].values())),
}
for key, field in (("objects_by_compile_flags", "compile_flags"), ("objects_by_observed", "observed")):
    for v in objects.values():
        label = "+".join(v[field]) or "none"
        summary[key][label] = summary[key].get(label, 0) + 1
json.dump({"summary": summary, "objects": objects, "artifacts": artifacts},
          open(out_json, "w"), indent=1, sort_keys=True)
print(json.dumps(summary, indent=1))
