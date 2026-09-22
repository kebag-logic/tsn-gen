# Remaining gates after the A162 author task (issue 15)

The author lane is complete and stopped. Everything below is owned by the manager or the named reviewers; none of it has been done or claimed by A162.

Candidate: `76906b83eb54ac29040fe72fd910d34727188694` (tree `a34065c39ea41aeb5daae9ecae3763f632f27484`) on local branch `15-instrument-sanitizer-targets`, parent main `deca300c9eb4863fa13383b45ba6cb6fd0828671`. Not pushed.

## Manager

1. Verify the local candidate identity before publication:
   `git -C $VALIDATION_STORAGE/lanes/tsngen15-sanitizer-targets rev-parse HEAD HEAD^{tree} HEAD^` must print the candidate, tree and base above. `git status --porcelain --ignored` must be empty, and `git log --format=%B -1` must be the single subject line without trailers.
2. Confirm live main is still `deca300c9eb4863fa13383b45ba6cb6fd0828671`, or decide on rebase/merge-candidate handling if it moved.
3. Publish the branch and open the PR with `PR-BODY.md` (it closes #15), then attach the public evidence the manager selects from this bundle (`MANIFEST.json` lists hashes).
4. Hosted CI on the published head and on the pull-request merge candidate, all existing jobs:
   - `Unit-testing` (`.github/workflows/unit-testing.yml`): `./setup.sh d`, then `./tests/run-tests.sh`. Local expectation: step exits 0, CTest 292/292 (285 existing + 7 `sanitizer_probe` cases).
   - `Behave-Testing` (`.github/workflows/behave-testing.yml`): `./setup.sh d`, then `./tests/run-tests-behave.sh`. Local expectation: AECP 96 scenarios / 395 steps passed with 2 scenarios / 8 steps skipped; stack 115 scenarios / 563 steps passed.
   - The hosted image differs from this lane: the issue 14 author evidence (https://github.com/kebag-logic/tsn-gen/issues/14#issuecomment-5583554254) records GNU 13.3.0 on Ubuntu 24.04, and the workflows initialize all submodules. Neither was available here. In the hosted default build the existing `_ASAN`/`_UBSAN` tests are now actually instrumented. An ASan finding would fail the job. A recovered UBSan report would not, and `--output-on-failure` does not print passing tests' output. If the hosted `Testing/Temporary/LastTest.log` cannot be retrieved, record that hosted UBSan recoveries are unobserved.
   - `setup.sh` does not propagate prep or unit-test failures (#17), so read the separate `./tests/run-tests.sh` step exit, not the setup step.
5. Local workflow replication, if the manager's bar requires it beyond commands 070 to 074 (which ran `setup.sh d`, `tests/run-tests.sh` and `tests/run-tests-behave.sh` literally on a fresh export, without the all-submodule checkout step).
6. Candidate, containment and merge bar per the standing policy, including the automatic merge after it passes.

## Reviewers R231 (internal) and R232 (external)

Independent review of the candidate against the frozen decision (https://github.com/kebag-logic/tsn-gen/issues/15#issuecomment-5772396945). `REVIEW-READY.md` lists the review entry points and fast reproduction commands.

## Explicitly not covered, to stay qualified in any publication

- CMake 3.21 minimum: not executed. The new CMake uses `add_test(NAME ... COMMAND ...)` with `$<TARGET_FILE:...>`, the `ENVIRONMENT` and `FAIL_REGULAR_EXPRESSION` test properties, and script-mode `execute_process` / `string(APPEND)`, all available before 3.21.
- Clang and llvm-symbolizer: not installed. The second-compiler control is compile-only (aarch64 GCC 16.1.0 without target sanitizer runtimes).
- tshark: absent, so `stack_codec_tshark` was not registered.
- `@hardware` and `@verilator` scenarios: excluded by the stock tags; no hardware or live-DUT acceptance.
- Legacy `docs/01_testing.md` still says a sanitizer error exits non-zero (untouched legacy prose; the tier pages govern per `docs/INDEX.md`).
