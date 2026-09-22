#!/usr/bin/env python3
"""Render COMMANDS.md from commands.jsonl (every recorded command, in order)."""
import json
import shlex

E = "$WORKSPACE_HOME/milan-fpga-management/2026-09-22/tsngen15-author"
HEAD = """# Commands (A162, tsn-gen issue 15)

Every command below was run through `scripts/run.sh`, which keeps its combined output in `logs/<id>.log` and appends argv, cwd, UTC start/end, the exact exit status and the log SHA-256 to `commands.jsonl`. Exits are the commands' own; expected failures (mutations, M5 links, the first 033 attempt) are marked in `HANDOFF.md`. Scratch root: `$VALIDATION_STORAGE/tmp/a162-tsngen15`; checkout: `$VALIDATION_STORAGE/lanes/tsngen15-sanitizer-targets`.

## Reproduction order

```bash
E=$WORKSPACE_HOME/milan-fpga-management/2026-09-22/tsngen15-author
S=$VALIDATION_STORAGE/tmp/a162-tsngen15
CO=$VALIDATION_STORAGE/lanes/tsngen15-sanitizer-targets
BASE=deca300c9eb4863fa13383b45ba6cb6fd0828671
CAND=76906b83eb54ac29040fe72fd910d34727188694
# submodules the build reads (external/CMakeLists.txt)
git -C $CO submodule update --init external/googletest
git -C $CO submodule update --init --recursive external/rapidyaml
# configure-only matrices (base captured from the checkout before the edit; candidate at CAND)
$E/scripts/configure-matrix.sh base $CO $S/cfg $E/receipts/configure    # checkout at BASE
$E/scripts/configure-matrix.sh cand $CO $S/cfg $E/receipts/configure    # checkout at CAND
python3 $E/scripts/compare-configure.py $E/receipts/configure base cand $E/receipts/compare-base-vs-candidate.json
# clean exports of the exact commits
$E/scripts/export-tree.sh $CO $BASE $S/src/base
$E/scripts/export-tree.sh $CO $CAND $S/src/cand
# default (setup.sh configuration) and documented Debug gates
$E/scripts/setup-equivalent.sh 010-base-default $S/src/base $S/build/base-default
$E/scripts/setup-equivalent.sh 031-cand-default $S/src/cand $S/build/cand-default
$E/scripts/docs-flow.sh 040-base-debug $S/src/base $S/build/base-debug -DCMAKE_BUILD_TYPE=Debug
$E/scripts/docs-flow.sh 040-cand-debug $S/src/cand $S/build/cand-debug -DCMAKE_BUILD_TYPE=Debug
$E/scripts/docs-flow.sh 043-cand-release $S/src/cand $S/build/cand-release -DCMAKE_BUILD_TYPE=Release
# (likewise MinSizeRel, RelWithDebInfo)
python3 $E/scripts/scan-test-log.py $S/build/cand-default $E/receipts/scan-cand-default.json
python3 $E/scripts/instrumentation.py $S/build/cand-default $E/receipts/instrumentation-cand-default.json
# mutations (patches in receipts/mutations/)
$E/scripts/mutation.sh 051-M1-base-helper-empty "M1 base helper" empty $E/receipts/mutations/M1-base-helper.patch
# BDD (disposable venv at $S/venv with behave), literal workflow replay, consumers, cross control
$E/scripts/bdd-suites.sh 062-bdd-cand-default $S/src/cand $S/build/cand-default/traffic-gen/packet_gen
(cd $S/src/replay-cand && ./setup.sh d; ./tests/run-tests.sh; ./tests/run-tests-behave.sh)
$E/scripts/consumer-controls.sh cand
$E/scripts/cross-compile-control.sh 090-cross-candidate $S/src/cand
python3 $E/scripts/summarize.py
```

## Recorded commands

| id | exit | start (UTC) | description | log |
| --- | --- | --- | --- | --- |
"""

rows = []
for line in open(f"{E}/commands.jsonl"):
    r = json.loads(line)
    desc = r["description"].replace("|", "\\|")
    rows.append(f"| {r['id']} | {r['exit']} | {r['start']} | {desc} | `{r['log']}` |")
with open(f"{E}/COMMANDS.md", "w") as f:
    f.write(HEAD + "\n".join(rows) + "\n\n## Full argv\n\n")
    for line in open(f"{E}/commands.jsonl"):
        r = json.loads(line)
        f.write(f"- `{r['id']}` (cwd `{r['cwd']}`): `{' '.join(shlex.quote(a) for a in r['argv'])}`\n")
print(len(rows), "commands")
