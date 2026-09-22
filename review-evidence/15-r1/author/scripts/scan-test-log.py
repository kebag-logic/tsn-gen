#!/usr/bin/env python3
"""Scan a CTest LastTest.log for sanitizer reports in every test's output.

Usage: scan-test-log.py <build-dir> [<out-json>]

UBSan recovers by default, so its reports can appear in passing tests'
output only. For every test (keyed by CTest index, name and executable,
since gtest names repeat across the plain/_ASAN/_UBSAN variants) this lists
each line matching a sanitizer report pattern, with the recorded result. The
probe's diagnostic controls are expected to contain reports and are listed
separately.
"""
import json
import os
import re
import sys
from collections import Counter

bld = sys.argv[1]
log = os.path.join(bld, "Testing", "Temporary", "LastTest.log")
pat = re.compile(r"runtime error:|Sanitizer|==\d+==ERROR")
tests = {}
key = None
for line in open(log, errors="replace").read().splitlines():
    m = re.match(r"^(\d+)/(\d+) Testing: (.+)$", line)
    if m:
        key = [int(m.group(1)), m.group(3).strip(), None]
        continue
    if key is not None and line.startswith("Command: "):
        argv = re.findall(r'"([^"]*)"', line)
        exe = os.path.basename(argv[0]) if argv else "?"
        if exe == "cmake" and any(a.startswith("-DCHILD=") for a in argv):
            exe = os.path.basename(next(a for a in argv if a.startswith("-DCHILD="))[8:])
        key[2] = exe
        k = f"{key[0]:03d} {key[1]} [{exe}]"
        tests[k] = {"name": key[1], "exe": exe, "result": None, "reports": []}
        continue
    if key is None or key[2] is None:
        continue
    k = f"{key[0]:03d} {key[1]} [{key[2]}]"
    m = re.match(r"^Test (Passed|Failed)", line)
    if m:
        tests[k]["result"] = m.group(1)
        continue
    if pat.search(line):
        tests[k]["reports"].append(line.strip())

def is_probe_diag(t):
    return t["name"].startswith("sanitizer_probe-test_") and not t["name"].endswith(".clean")

by_exe = Counter(t["exe"] for t in tests.values())
summary = {
    "log": log,
    "tests": len(tests),
    "passed": sum(1 for t in tests.values() if t["result"] == "Passed"),
    "not_passed": sorted(k for k, t in tests.items() if t["result"] != "Passed"),
    "tests_per_executable": dict(sorted(by_exe.items())),
    "tests_with_reports_outside_probe_diagnostics": {
        k: t["reports"] for k, t in sorted(tests.items()) if t["reports"] and not is_probe_diag(t)},
    "probe_diagnostic_reports": {
        k: [r for r in t["reports"] if "runtime error" in r or "ERROR:" in r]
        for k, t in sorted(tests.items()) if is_probe_diag(t)},
}
print(json.dumps(summary, indent=1))
if len(sys.argv) > 2:
    with open(sys.argv[2], "w") as f:
        json.dump(summary, f, indent=1)
