#!/usr/bin/env python3
"""Collect the key author results from receipts/ and commands.jsonl into
receipts/results-summary.json (no new measurement; every value is read from
an existing receipt or command record)."""
import json
import os
import re

E = "$WORKSPACE_HOME/milan-fpga-management/2026-09-22/tsngen15-author"
R = os.path.join(E, "receipts")
cmds = {}
for line in open(os.path.join(E, "commands.jsonl")):
    rec = json.loads(line)
    cmds[rec["id"]] = rec


def exit_of(cid):
    return cmds[cid]["exit"] if cid in cmds else None


def ctest_counts(cid):
    text = open(os.path.join(E, "logs", cid + ".log")).read()
    m = re.search(r"(\d+)% tests passed, (\d+) tests failed out of (\d+)", text)
    if m:
        return {"failed": int(m.group(2)), "total": int(m.group(3))}
    m = re.search(r"100% tests passed out of (\d+)", text)
    return {"failed": 0, "total": int(m.group(1))} if m else None


def behave_counts(cid):
    text = open(os.path.join(E, "logs", cid + ".log")).read()
    out = {}
    for kind in ("features", "scenarios", "steps"):
        m = re.search(r"(\d+) %s passed, (\d+) failed, (\d+) skipped" % kind, text)
        out[kind] = {"passed": int(m.group(1)), "failed": int(m.group(2)), "skipped": int(m.group(3))}
    return out


summary = {"candidate": open(os.path.join(R, "commit.txt")).read().split("\n")[0].split()[1]}

comp = json.load(open(os.path.join(R, "compare-base-vs-candidate.json")))
summary["generated_commands"] = {
    cfg: {kind: {"classes": c[kind]["classes"], "new_in_candidate": len(c[kind]["only_in_b"]),
                 "missing_in_candidate": len(c[kind]["only_in_a"])}
          for kind in ("compile", "link", "flags")}
    for cfg, c in comp["configurations"].items()}

inst = {}
for name in sorted(os.listdir(R)):
    m = re.match(r"instrumentation-(.+)\.json$", name)
    if m:
        s = json.load(open(os.path.join(R, name)))["summary"]
        inst[m.group(1)] = {
            "objects": s["objects"],
            "by_compile_flags": s["objects_by_compile_flags"],
            "by_observed_references": s["objects_by_observed"],
            "flag_vs_reference_mismatches": sorted(s["flag_vs_observed_mismatches"]),
            "runtime_linked_artifacts": len(s["artifacts_linking_a_runtime"]),
        }
summary["instrumentation_and_linkage"] = inst

scans = {}
for name in sorted(os.listdir(R)):
    m = re.match(r"scan-(.+)\.json$", name)
    if m:
        s = json.load(open(os.path.join(R, name)))
        scans[m.group(1)] = {
            "tests": s["tests"], "passed": s["passed"],
            "tests_with_sanitizer_reports_outside_probe_diagnostics":
                len(s["tests_with_reports_outside_probe_diagnostics"]),
            "probe_diagnostics_with_report": sum(1 for v in s["probe_diagnostic_reports"].values() if v),
        }
summary["test_output_scans"] = scans

summary["unit_gates"] = {
    cid: {"exit": exit_of(cid), "ctest": ctest_counts(cid)}
    for cid in ("010-base-default-unit", "031-cand-default-unit", "040-base-debug-unit",
                "040-cand-debug-unit", "043-cand-release-unit", "043-cand-minsizerel-unit",
                "043-cand-relwithdebinfo-unit", "072-replay-setup", "073-replay-unit")}
summary["behave"] = {
    cid: {"exit": exit_of(cid), "counts": behave_counts(cid)}
    for cid in ("062-bdd-cand-default-aecp_behave", "062-bdd-cand-default-stack_behave",
                "062-bdd-base-default-aecp_behave", "062-bdd-base-default-stack_behave")}
summary["behave"]["074-replay-behave"] = {"exit": exit_of("074-replay-behave")}

muts = {}
for cid in sorted(c for c in cmds if re.match(r"05\d-M\d", c) and c.endswith("-ctest")):
    text = open(os.path.join(E, "logs", cid + ".log")).read()
    res = dict(re.findall(r"Test +#\d+: (\S+) \.+\s*(\*\*\*Failed|Passed)", text))
    muts[cid[:-len("-ctest")]] = {"ctest_exit": exit_of(cid),
                                  "failed": sorted(k for k, v in res.items() if v != "Passed"),
                                  "passed": sorted(k for k, v in res.items() if v == "Passed")}
muts["056-M5-no-runtime-at-link-empty"] = {
    "asan_build_exit": exit_of("056-M5-no-runtime-at-link-empty-build"),
    "ubsan_build_exit": exit_of("057-M5-no-runtime-at-link-ubsan-build")}
summary["mutations"] = muts

cross = {}
for lab in ("090-cross-candidate", "091-cross-M1-base-helper"):
    objs = json.load(open(os.path.join(R, "cross", lab, "objects.json")))
    cross[lab] = {
        "objects": len(objs),
        "with_flag": sum(1 for v in objs.values() if v["compile_flags"]),
        "with_references": sum(1 for v in objs.values() if v["asan_refs"] or v["ubsan_refs"]),
        "flagged_without_references": sorted(k for k, v in objs.items()
                                             if v["compile_flags"] and not (v["asan_refs"] or v["ubsan_refs"])),
        "variant_libs_build_exit": exit_of(lab + "-libs"),
        "sanitized_executable_link_exit": exit_of(lab + "-exe-link"),
    }
summary["second_compiler_compile_only"] = cross

cons = json.load(open(os.path.join(R, "compare-consumers-base-vs-candidate.json")))
summary["consumer_controls"] = {
    cfg: {kind: c[kind]["classes"] for kind in ("compile", "link", "flags")}
    for cfg, c in cons["configurations"].items()}
summary["consumer_controls_exits"] = {c: exit_of(c) for c in sorted(cmds) if c.startswith(("080-", "081-"))}

json.dump(summary, open(os.path.join(R, "results-summary.json"), "w"), indent=1, sort_keys=True)
print(json.dumps(summary, indent=1, sort_keys=True))
