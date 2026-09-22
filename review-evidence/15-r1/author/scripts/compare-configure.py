#!/usr/bin/env python3
"""Compare two configure-matrix captures (see configure-matrix.sh).

Usage: compare-configure.py <capture-root> <label-a> <label-b> <out-json>

For every <cfg>-shared_<x> directory present under both labels:
  * compile commands are keyed by object output path; absolute build and
    source roots are neutralised to $BUILD / $SRC before comparison;
  * each command pair is classified as identical, or as differing by exactly
    one added -fsanitize=address / -fsanitize=undefined token (reported with
    its target), or as any other difference;
  * every link.txt and flags.make file is compared the same way;
  * the instrumented compile population of both labels is listed per
    sanitizer, with the plain (uninstrumented) population counted.
Prints a readable summary and writes the full result as JSON.
"""
import json
import os
import re
import shlex
import sys
from collections import Counter

root, label_a, label_b, out_json = sys.argv[1:5]
SAN = {"-fsanitize=address": "address", "-fsanitize=undefined": "undefined"}


def roots_from_log(capdir):
    argv = open(os.path.join(capdir, "configure.argv")).read()
    toks = shlex.split(argv)
    src = toks[toks.index("-S") + 1]
    bld = toks[toks.index("-B") + 1]
    # the embedded consumer names the tsn-gen tree separately
    tsn = [t.split("=", 1)[1] for t in toks if t.startswith("-DTSN_GEN_SOURCE_DIR=")]
    if tsn:
        src = (src, tsn[0])
    return src, bld


def neutral(text, src, bld):
    text = text.replace(bld, "$BUILD")
    if isinstance(src, tuple):
        return text.replace(src[1], "$TSNSRC").replace(src[0], "$SRC")
    return text.replace(src, "$SRC")


def target_of(output):
    m = re.search(r"CMakeFiles/([^/]+)\.dir/", output)
    return m.group(1) if m else output


def load_compile(capdir):
    src, bld = roots_from_log(capdir)
    entries = json.load(open(os.path.join(capdir, "compile_commands.json")))
    res = {}
    for e in entries:
        out = neutral(e["output"], src, bld)
        res[out] = neutral(e["command"], src, bld)
    return res


def load_files(capdir, sub):
    src, bld = roots_from_log(capdir)
    base = os.path.join(capdir, sub)
    res = {}
    for dp, _, fns in os.walk(base):
        for fn in fns:
            p = os.path.join(dp, fn)
            res[os.path.relpath(p, base)] = neutral(open(p).read(), src, bld)
    return res


def sanitizers(cmd):
    return sorted({SAN[t] for t in cmd.split() if t in SAN})


def classify(a, b):
    if a == b:
        return "identical", None
    ta, tb = a.split(), b.split()
    added = Counter(tb) - Counter(ta)
    removed = Counter(ta) - Counter(tb)
    if not removed and sum(added.values()) == 1:
        tok = next(iter(added))
        if tok in SAN:
            # also require the remaining token order to be unchanged
            rest = list(tb)
            rest.remove(tok)
            if rest == ta:
                return "added:" + tok, None
    return "other", {"added": sorted(added.elements()),
                     "removed": sorted(removed.elements())}


cfgs = sorted(set(os.listdir(os.path.join(root, label_a))) &
              set(os.listdir(os.path.join(root, label_b))))
result = {"label_a": label_a, "label_b": label_b, "configurations": {}}
for cfg in cfgs:
    ca = os.path.join(root, label_a, cfg)
    cb = os.path.join(root, label_b, cfg)
    ra = {"compile": load_compile(ca), "link": load_files(ca, "link"),
          "flags": load_files(ca, "flags")}
    rb = {"compile": load_compile(cb), "link": load_files(cb, "link"),
          "flags": load_files(cb, "flags")}
    cres = {}
    for kind in ("compile", "link", "flags"):
        a, b = ra[kind], rb[kind]
        common = sorted(set(a) & set(b))
        cls = Counter()
        detail = {}
        for k in common:
            c, d = classify(a[k], b[k])
            cls[c] += 1
            if c != "identical":
                detail[k] = {"class": c, "target": target_of(k)}
                if d:
                    detail[k]["diff"] = d
        cres[kind] = {
            "count_a": len(a), "count_b": len(b),
            "only_in_a": sorted(set(a) - set(b)),
            "only_in_b": sorted(set(b) - set(a)),
            "classes": dict(cls), "changed": detail,
        }
    pop = {}
    for lab, r in ((label_a, ra), (label_b, rb)):
        by = {"address": [], "undefined": [], "none": []}
        for out, cmd in sorted(r["compile"].items()):
            s = sanitizers(cmd)
            if len(s) > 1:
                by.setdefault("both", []).append(out)
            elif s:
                by[s[0]].append(out)
            else:
                by["none"].append(out)
        pop[lab] = {k: (v if k != "none" else len(v)) for k, v in by.items()}
        pop[lab]["link_with_sanitizer"] = {
            k: sanitizers(v) for k, v in sorted(r["link"].items()) if sanitizers(v)}
    cres["population"] = pop
    result["configurations"][cfg] = cres

with open(out_json, "w") as f:
    json.dump(result, f, indent=1, sort_keys=True)

for cfg, c in result["configurations"].items():
    print(f"== {cfg}")
    for kind in ("compile", "link", "flags"):
        k = c[kind]
        print(f"  {kind}: a={k['count_a']} b={k['count_b']} classes={k['classes']}"
              f" only_in_a={len(k['only_in_a'])} only_in_b={len(k['only_in_b'])}")
        others = {p: v for p, v in k["changed"].items() if v["class"] == "other"}
        for p, v in sorted(others.items()):
            print(f"    OTHER {p}: {v.get('diff')}")
        added_targets = Counter(v["target"] + " " + v["class"]
                                for v in k["changed"].values() if v["class"] != "other")
        for t, n in sorted(added_targets.items()):
            print(f"    {t} x{n}")
        for p in k["only_in_b"]:
            print(f"    new in b: {p}")
        for p in k["only_in_a"]:
            print(f"    missing in b: {p}")
    for lab in (label_a, label_b):
        p = c["population"][lab]
        print(f"  population {lab}: address={len(p['address'])} undefined={len(p['undefined'])}"
              f" none={p['none']} both={len(p.get('both', []))}"
              f" sanitizer-linked={len(p['link_with_sanitizer'])}")
