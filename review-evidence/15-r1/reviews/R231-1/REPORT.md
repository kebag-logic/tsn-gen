[R231] POSITIVE - exact head 76906b83eb54ac29040fe72fd910d34727188694

# R231-1 cold internal review: kebag-logic/tsn-gen PR #18 (issue #15)

| Item | Value |
| --- | --- |
| Reviewer / round | R231 (cold internal Opus), round R231-1. R231 did not author the change and had no private author context. |
| Head (reviewed) | `76906b83eb54ac29040fe72fd910d34727188694`, tree `a34065c39ea41aeb5daae9ecae3763f632f27484`, single parent `deca300c9eb4863fa13383b45ba6cb6fd0828671` |
| Base / live main | `deca300c9eb4863fa13383b45ba6cb6fd0828671`. `gh api repos/kebag-logic/tsn-gen/branches/main` still returned this SHA during the review (2026-09-22, ~11:25 +02:00). |
| Commit metadata | 1 commit, author and committer `hackerman-kl`, subject only: "Instrument sanitizer variants and their test TUs under every build type". PR #18 is open and not a draft; `mergeable_state` was `clean`; it has 0 reviews and 0 review comments. |
| Authority | Issue #15 body; frozen READY DECISION https://github.com/kebag-logic/tsn-gen/issues/15#issuecomment-5772396945 (A10); A162 TAKEN (5772572984) and candidate (5772855876) comments; PR #18 comments 5773121792, 5773411180, 5773918137 (A10). |
| Public evidence read | Tree `0be5bf0dc34bd7924b4227c1c2a2602849db4c52` `review-evidence/15-r1/`. Selected blobs were fetched and their git blob SHA-1 verified (`receipts/public-evidence/`, `receipts/fetch_evidence.py`). |
| Verdict | **POSITIVE.** There are 0 BLOCKER, 0 MAJOR and 0 MINOR findings. The 2 SUGGESTIONs (S1, S2) are optional documentation precision points. All five lenses have clean coverage at this head (see the ledger). |

Path tokens used below: `<CLONE>` = `$VALIDATION_STORAGE/reviews/r231-18-r1` (the given detached checkout), `<SCRATCH>` = `$VALIDATION_STORAGE/reviews/r231-18-r1-scratch` (R231 disposable copies and builds), `<OUT>` = `$WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r231`. Raw receipts under `<OUT>/receipts` keep absolute paths. `receipts/commands.jsonl` records the argv, cwd, env overrides, exit status, duration and log SHA-256 of all 121 commands run through `receipts/rr.py`.

## 1. What the change does

- `cmake/CMakeGenTestingLibraries.cmake:16-38` moves `${COMP_ASAN_FLAGS}` / `${COMP_UBSAN_FLAGS}` out of the four `$<CONFIG:...>` expressions. The flag stays in the same PUBLIC `target_compile_options` call. The per-configuration `-O`/`-g` lists and `COMP_COVERAGE_FLAGS` placement are unchanged.
- `CMakeLists.txt:63-66` adds `tests/sanitizer` only under `ENABLE_PARSER_TESTS`.
- New `tests/sanitizer/` contents:
  - a fixture library built by `generate_test_libs`;
  - a probe built by `target_link_testlibs`;
  - 7 CTest cases: 3 clean, 2 ASan diagnostics and 2 UBSan diagnostics. The diagnostics run through `check_sanitizer_child.cmake`, and only the two UBSan diagnostics set `UBSAN_OPTIONS=halt_on_error=1`.
- Docs: `docs/low-level/testing-and-ci.md` and `docs/low-level/architecture.md`.
- No protocol YAML, stack, component source or existing test source changes. `git diff --name-status` lists exactly 9 paths (`receipts/99-final-state.txt`).

## 2. Independent evidence (R231-executed unless marked otherwise)

Environment:

- Host tools: Arch Linux x86_64, GCC 16.2.1, CMake 4.4.3, GNU Make 4.4.1, Ninja 1.13.2 and 8 CPUs. All builds used at most 8 jobs.
- Absent: Clang, llvm-symbolizer and tshark.
- Sources: base and candidate were exported with `git archive` into `<SCRATCH>/src/*`. The googletest/rapidyaml submodules were included at their pinned commits, which are identical in base and head. Every exported file was verified by git blob SHA-1 and mode: 0 mismatches in 1,212 base and 1,217 candidate entries, superproject plus submodules (`receipts/02-export-*.txt`, `receipts/export_tree.py`).
- Submodules: googletest and rapidyaml (recursive) were initialized in `<CLONE>` from their public origins.
- Environment variables: `CMAKE_*`, `CC`/`CXX`/`*FLAGS` and `*SAN_OPTIONS` were all unset.

### E1. Generated compile/link population: 5 build types × `BUILD_SHARED_LIBS` {unset, ON}, base vs candidate (configure-only, Unix Makefiles)

Receipts: `receipts/11-compare-base-cand.{txt,json}`, `receipts/11-compare-examples.txt`, script `receipts/compare_cfg.py`, logs `logs/10-configure-*`; all 20 configures exited 0.

- **Debug, Release, MinSizeRel, RelWithDebInfo** (both shared settings): all 125 existing compile commands, 41 `link.txt` and 41 `flags.make` are byte-identical to base after path neutralization. The probe adds 6 targets (6 compile entries, 6 `link.txt`, 6 `flags.make`).
- **Empty type** (both shared settings):
  - 67 existing compile commands are identical. Each of the other 58 differs from base only by one inserted token, the matching flag. That was checked by token-level diff, and the flag was absent in base.
  - The 58 are `protocol_parser_{asan,ubsan}` (11 each), `traffic_gen_{asan,ubsan}` (10 each), and the 8 `_ASAN` plus 8 `_UBSAN` test TUs.
  - All 41 existing `link.txt` are identical. Base's `ptp_flags-test_ASAN` compiled without `-fsanitize` while linking with `-fsanitize=address`, which is exactly issue #15's observation. The candidate adds `-fsanitize=address` to that compile command.
- `-fsanitize` compile entries, base → candidate: empty type 0 → 62 (31+31); each standard type 58 → 62.
- No candidate compile entry outside `*_asan/_ubsan/_ASAN/_UBSAN` targets carries `-fsanitize` (`unexpected_flagged_cand` is empty).
- No compile command in any configuration contains `sanitize-recover`, `fno-sanitize` or `sanitize-trap` (`receipts/80-install-export-recover.txt`).

### E2. Compiler instrumentation vs runtime linkage (built objects/artifacts, `nm -u`, `readelf -d`)

Receipts: `receipts/22-instrumentation-{base,cand}-default.{txt,json}`, `receipts/27-instrumentation-cand-*.json`, `receipts/27-instrumentation-cand-standard-types.txt`, script `receipts/instrumentation.py`.

- **Base, default (empty) build** (`setup.sh d`):
  - 0 of the 58 variant/test objects reference any `__asan_*` or `__ubsan_handle_*` entry point.
  - Yet 16 test executables record `libasan.so.8` / `libubsan.so.1` as NEEDED. The runtime is linked, but nothing is instrumented.
- **Candidate, empty type**:
  - All 31 ASan-flagged objects reference `__asan_*`.
  - 30 of the 31 UBSan-flagged objects reference `__ubsan_handle_*`. The exception is `parser/src/db_proto_impl.cpp`, a documented placeholder TU with no code. It is compiled with the flag (E1), and its object holds only constants.
  - None of the 69 plain, `protocol_logic`, `packet_gen`, ryml/c4core or gtest/gmock objects references a sanitizer.
  - 18 executables have a sanitizer NEEDED entry: the 16 existing ones and 2 from the probe.
- **Candidate, Debug / Release / MinSizeRel / RelWithDebInfo** (full builds, exit 0; Release builds with `-Werror -pedantic`): the same figures hold, 31/31, 30/31 (same placeholder), 0 plain, 18 NEEDED.
- The six variant `.so` files leave their sanitizer symbols undefined without a NEEDED entry. They are resolved from the executable's runtime, the same link arrangement as base's configured types.
- The NEEDED and RUNPATH entries of all 41 existing ELF artifacts are identical between base and candidate (`receipts/26-needed-compare.txt`).

### E3. Default workflow and the direct unit verdict

Receipts: `logs/20-*-setup-d.log`, `logs/21-*`, `logs/24-*`.

The literal `bash setup.sh d` was run in disposable base and candidate copies, followed by a separate `bash tests/run-tests.sh`. Both `setup.sh` runs exited 0 with 0 compiler warnings.

| Direct runner run | Base | Candidate |
| --- | --- | --- |
| First (`logs/21-*`) | exit 0, 285/285 | exit 0, 292/292 |
| Rerun (`logs/24-*`) | exit 0, 285/285 | exit 0, 292/292 |

The rerun was needed because `ctest --show-only` overwrites `LastTest.log`. The verdict comes from the direct runner's exit status, not from `setup.sh`, whose last command is `cd`.

### E4. Registration parity

Receipts: `receipts/23-ctest-registration-{base,cand}-default.json`, `receipts/23-ctest-registration-compare.txt`.

From `ctest --show-only=json-v1` on the built default trees:

- The first 285 candidate registrations equal base in order, including name, full command and properties.
- The 7 probe cases are appended.
- The `ENVIRONMENT` property appears on exactly two of the 292 tests: `sanitizer_probe-test_UBSAN.{library,consumer}-signed-overflow` = `UBSAN_OPTIONS=halt_on_error=1`.
- The three clean cases carry `FAIL_REGULAR_EXPRESSION "Sanitizer|runtime error"`.
- No `*SAN_OPTIONS` appears in any other CTestTestfile. The default build has `CMAKE_BUILD_TYPE=''` and `CMAKE_CXX_FLAGS=''`.

### E5. Reports in passing output

Receipts: `receipts/24-scan-reports.txt`, `receipts/24-LastTest-{base,cand}-default.log`, `receipts/25-probe-diagnostic-blocks.txt`, script `receipts/scan_lasttest.py`.

- `LastTest.log` retains output for every passing test: 285/285 base and 292/292 candidate.
- In the candidate default run, exactly 4 blocks contain sanitizer report text: the 4 intentional probe diagnostics. None of the 285 existing tests' output contains `runtime error:` or `*Sanitizer`, so GCC 16.2.1 shows no recovered UBSan report in any existing test.
- The diagnostic blocks record `child exit status: 1`, the options in effect, the child's stdout ending at `... started`, and its full stderr report.
- The ASan report names `sanitizer_probe::libraryRead` in `libsanitizer_probe_asan.so`. The UBSan report names `sanitizer_probe_test.cpp:34:18` for the consumer case.

### E6. Probe controls in all five build types, unmodified candidate (probe targets only)

Receipts: `receipts/mutations/M0-none-*`, `logs/3x-M0-*`.

The 7 cases pass 7/7 in empty, Debug, Release, MinSizeRel and RelWithDebInfo; every ctest exit is 0. In the empty type, the probe objects reference ASan in the library (3) and the `_ASAN` test TU (12), and UBSan in the library (3) and the `_UBSAN` test TU (5). The plain probe has none.

### E7. Mutation controls (disposable candidate copies)

Script: `receipts/mutate.py` (each anchor asserted unique) and `receipts/run_mutation.sh`. Per-mutation receipts: `receipts/mutations/<id>/{mutation.diff,summary.txt,LastTest.log,instrumentation.json}`. Failure reasons: `receipts/mutations/failure-reasons.txt`.

| ID | Mutation | Type | Result (ctest exit) | Failing cases and recorded reason |
| --- | --- | --- | --- | --- |
| M1 | base `CMakeGenTestingLibraries.cmake` restored (instrumentation reverted) | empty | 3/7 pass (8) | All 4 diagnostics fail. Each child exited 0 with no report and printed `completed`. The probe objects have 0 sanitizer references, yet `_ASAN`/`_UBSAN` still NEED `libasan.so.8`/`libubsan.so.1`: runtime linked, no instrumentation. |
| M1 | same | Debug, Release | 7/7 (0) | Base helper instruments the configured types, so the defect depends on build type. |
| M1 | same, CMake 3.21.4 | empty | 3/7 (8) | Same 4 diagnostics fail (`logs/72-*`). |
| M2 | sanitizer flags PRIVATE (library only) | empty, Release | 5/7 (8) | Only the `consumer-*` ASan/UBSan cases fail (child exit 0, no report, `completed`). The library objects are instrumented; the test TUs have 0 references. |
| M3 | sanitizer flags INTERFACE (consumer only) | empty, Release | 5/7 (8) | Only the `library-*` cases fail. The test TUs are instrumented; the library objects have 0 references. |
| M4 | `halt_on_error` ENVIRONMENT removed | empty | 5/7 (8) | Only the 2 UBSan diagnostics fail. The child prints its `runtime error`, prints `completed` and exits 0; a stderr match alone is not accepted. |
| M5 | link-time `-fsanitize` removed from `target_link_testlibs` | empty | build exit 2 | Link fails on 12 undefined `__asan_*` references. Runtime linkage is a separate, required step. |
| M6a | only the ASan flag reverted to the config expressions | empty | 5/7 (8) | Only the 2 ASan diagnostics fail. |
| M6b | only the UBSan flag reverted | empty | 5/7 (8) | Only the 2 UBSan diagnostics fail. |
| M7 | the 4 faulting modes run under the clean-control policy | empty | 7 pass + 4 extra fail (8) | All 4 fail with `Error regular expression found in output. Regex=[Sanitizer\|runtime error]`, including the UBSan cases whose child exited 0. The clean controls would catch a report. |
| M8 | `_ASAN`/`_UBSAN` probes link the plain library instead of the variant | empty | 3/7 (8) | All 4 diagnostics fail. |

### E8. Checker honesty (`check_sanitizer_child.cmake` run directly)

Receipts: `logs/40-checker-*.log`.

| Child | Checker exit | Reason recorded |
| --- | --- | --- |
| Faithful fake (started, report, exit 1) | 0 | as expected |
| Real ASan child, default options | 0 | `child exit status: 1` |
| Report but exit 0 | 1 | `exit status 0, expected 1` |
| Exit 1, no report | 1 | no matching report |
| Report then `completed` | 1 | `continued past` |
| Report, killed by SIGSEGV | 1 | `exit status Segmentation fault` |
| Report without `started` | 1 | `never reached` |
| Real child with `ASAN_OPTIONS=exitcode=23` | 1 | exit 23 recorded, not normalized |
| Real child with `UBSAN_OPTIONS=halt_on_error=1:exitcode=7` | 1 | exit 7 recorded |
| UBSan report from the wrong TU against the library regex | 1 | no matching report |

The checker prints the inherited `ASAN_OPTIONS`/`UBSAN_OPTIONS` in every case.

### E9. Consumers and other configurations

Receipts: `receipts/51-compare-consumers.txt`, `logs/50-*`, `logs/52-*`, `receipts/60-extra-configs.txt`, `receipts/71-cmake321-summary.txt`, `receipts/70-cmake-3.21.4-wheel.sha256`.

- **Tests-off top level** (`ENABLE_PARSER_TESTS=OFF`, empty type, `BUILD_SHARED_LIBS=ON`):
  - Generated compile commands (52) and `link.txt`/`flags.make` (12) are identical to base.
  - There is no `-fsanitize` anywhere and no tests are registered.
  - The candidate built and `packet_gen --validate` ran, both exit 0.
- **Embedded `add_subdirectory` consumer** (tests default OFF), empty and Debug:
  - Generated files are identical to base (53 compile entries, 14 files) with no `-fsanitize`.
  - The candidate's empty-type app built (0 warnings) and ran (`parse ok`, exit 0).
- **Embedded consumer forcing `ENABLE_PARSER_TESTS=ON`** (configure-only): configures, with 31+31 flagged entries and the 7 probe tests.
- **Custom `CMAKE_BUILD_TYPE=Profile`** (configure-only): base 0/0 flagged, candidate 31/31.
- **Ninja Multi-Config** (configure-only): Debug, Release and RelWithDebInfo each have 31 ASan and 31 UBSan flagged objects (base 29/29), none outside the variants.
- **CMake 3.21.4**: a PyPI wheel (SHA-256 recorded) in a scratch venv, run on this host with GCC 16.2.1. Candidate empty type: configure 0 warnings, build exit 0, 292/292 pass, 31/31 flagged. M1 under the same CMake fails the 4 diagnostics.

### E10. Install, export and whitespace

Receipts: `receipts/80-install-export-recover.txt`.

- `cmake --install` of the default builds gives the same 179-file set in base and candidate. No probe or variant library is installed, and non-binary installed files are byte-identical.
- The generated `tsn-genTargets*.cmake` files are identical.
- `git diff --check deca300c..76906b83` exits 0.

### E11. Hosted gates (read-only public GitHub data; not rerun by R231)

Receipts: `receipts/hosted/{check-runs.json,runs.json,main-sha.txt,job-*.log,hosted-summary.txt}`.

- 4 check runs at the head, all `success`: Unit-testing and Behave-Testing, each on push and on pull_request.
- Hosted toolchain: `ubuntu-24.04`, GNU 13.3.0. The pull_request jobs checked out merge commit `60be6b6` (Merge 76906b8 into deca300).
- Unit jobs: `./setup.sh d` and then a separate `./tests/run-tests.sh` step. Both ctest runs in each job report 292/292, including all 7 probe cases.
- Behave jobs: 96 scenarios passed with 2 skipped, and 115 passed.
- Because the probe diagnostics pass on hosted GCC 13.3 in the default configuration, compiler instrumentation is shown on the hosted toolchain too, not just runtime linkage. That follows from E7/M1: without instrumentation these cases fail.

### Evidence by others, attributed and not rerun by R231

- **A10 (manager)**, public `manager/full-native/results.json` plus logs 02/03/04/06, blob hashes verified: the 7 native commands exit 0.
  - Default and explicit Debug direct runner: 292/292 each.
  - BDD: 8 features / 96 scenarios / 395 steps passed (2 scenarios / 8 steps skipped), and 4 / 115 / 563 passed.
  - Toolchain: GNU 16.2.1.
- **A162 (author)**: the matrix, mutation and consumer receipts under `author/`. R231 reproduced its central claims independently (E1–E9). The aarch64 cross-compile control was not reproduced by R231.

## 3. Findings

No BLOCKER, MAJOR or MINOR findings.

### S1 — SUGGESTION — lenses: Docs, Conformance

- **Artifact.** Two passages state UBSan's default behavior more broadly than it is:
  - `docs/low-level/testing-and-ci.md:63-65`: "UBSan recovers by default from most checks: it prints the report to stderr and the program carries on, so a UBSan report fails a test only when the test fails for another reason."
  - `docs/low-level/architecture.md:195-196`: "UBSan recovers by default, printing its report and carrying on."
- **Requirement.** The frozen decision says to "describe actual normal test semantics accurately" and "record exact diagnostic/exit behavior".
- **Evidence.** `receipts/90-ubsan-default-recovery-semantics.{cpp,txt}` uses the project's own `-fsanitize=undefined` with GCC 16.2.1 and no `UBSAN_OPTIONS`:
  - signed overflow: report, execution continues, exit 0;
  - `return` check (flowing off the end of a value-returning function): report, exit 1;
  - `unreachable` check: report, exit 1.

  GCC documents `-fsanitize=return` and `-fsanitize=unreachable` as the UBSan checks that do not recover. For comparison, `receipts/91-asan-leak-exit-semantics.*` confirms the ASan wording, since even an at-exit leak report exits 1.
- **Impact.** Low.
  - The testing page qualifies recovery with "most checks", so the recovery default the issue is about is described correctly.
  - The consequence clause and the unqualified architecture sentence overstate it: a report from those two checks fails a test by itself.
  - No defect can be hidden by this wording, and test behavior is unaffected.
- **Required outcome (optional).** Qualify both sentences. For example: "UBSan recovers by default from most checks (`return` and `unreachable` always stop with exit status 1) … so a recoverable UBSan report does not fail a test by itself." The architecture sentence could get the same qualifier or defer to testing-and-ci.md.
- **Verification.** Re-read the two passages. Optionally rerun the 3-mode program.

### S2 — SUGGESTION — lenses: Docs, Tests

- **Artifact.** `docs/low-level/testing-and-ci.md:66-67` advises looking for `runtime error:` in output, with passing output kept in `LastTest.log`. The `tests/sanitizer` diagnostic controls (`tests/sanitizer/CMakeLists.txt:48-70`) print such reports intentionally.
- **Requirement.** The docs should support the documented log-scanning workflow without false alarms. This is a usability point, not an acceptance item.
- **Evidence.** `receipts/24-scan-reports.txt`: in a clean candidate run, the only 4 passing tests whose retained output contains sanitizer report text are the probe diagnostics. Two contain `runtime error:` and two contain `ERROR: AddressSanitizer`.
- **Impact.** Low. A scan that follows the doc always matches these 4 expected reports, which could cause a false alarm or confusion. Nothing is masked.
- **Required outcome (optional).** One clause in the same paragraph saying that the four `sanitizer_probe-test_*.{library,consumer}-*` controls print reports on purpose and should be excluded from such scans.
- **Verification.** Re-read the paragraph.

Observations (not findings, no action required for this PR):

- Legacy `docs/01_testing.md:80-81` still says "A sanitiser error causes the test binary to exit non-zero". This is pre-existing and untouched, `docs/INDEX.md` ranks the tier pages above it, and A162 disclosed it.
- The ASan diagnostic regexes are TU-agnostic. TU specificity comes from each mode containing exactly one out-of-bounds access, as M2 and M3 confirm.
- CI builds only the empty type. The probe checks the helpers under whichever type is configured; R231 covered the other four types in this review (E1, E2, E6).
- The CMake comment and docs say only `-O`/`-g` follow the configuration. `COMP_COVERAGE_FLAGS`, currently empty, also stays per-configuration, as settled in TAKEN assumption 4. The wording is accurate in effect today.

## 4. Clean lens results

- **Conformance** (clean; S1 optional). Each frozen-decision element was checked against evidence:
  - Instrumentation is independent of build type, including empty and custom: E1, E2, E9.
  - Per-configuration optimization and debug levels, existing linkage and registrations are unchanged: E1, E2 (NEEDED/RUNPATH), E4.
  - Ordinary, embedded and tests-off consumers are unchanged: E9.
  - No global build type or sanitizer, no recovery-policy change, and no newly sanitized `protocol_logic` or third-party code: E1, E4, E10.
  - Real helper-built normal and diagnostic controls that detect removed instrumentation: E6, E7.
  - The fatal UBSan setting is explicit and recorded, and stderr is never turned into a synthetic child exit: E4, E8.
  - Direct runner exits are captured: E3.
  - Default and Debug native runs, both BDD suites and all hosted jobs: E3 and E11, plus A10.
  - Protocol, frame and assertion sources are untouched: section 1.
  - Artifacts: `cmake/CMakeGenTestingLibraries.cmake`, `CMakeLists.txt`, `tests/sanitizer/*`, the issue and decision comments, and receipts 11/22/23/24/26/27/51/60/80/hosted.
- **RTL / software and build architecture** (clean). Checked:
  - The flag is an unconditional PUBLIC compile option on the variants only.
  - Runtime linkage stays in `target_link_testlibs`, and `link.txt` is unchanged (E1).
  - The probe is built only by the real helpers and gated by `ENABLE_PARSER_TESTS`.
  - Only 6 new targets appear, none installed or exported (E10).
  - Variant `.so` runtime resolution is unchanged (E2).
  - Behavior under multi-config and custom types (E9).
  - Artifacts: `cmake/*.cmake`, `tests/sanitizer/CMakeLists.txt`, receipts 11/26/27/60/80.
- **Robustness** (clean). Checked:
  - Build types: 5 standard plus custom; `BUILD_SHARED_LIBS` both settings.
  - Generators: Makefiles (built) and Ninja Multi-Config (configure).
  - CMake 4.4.3 and 3.21.4 (wheel).
  - Tests-off, embedded, and embedded-with-tests-on consumers.
  - Release with `-Werror -pedantic`.
  - `volatile` operands across -O levels: M0 in all types, M2/M3 in Release.
  - Checker behavior with signals, altered exit codes, inherited options and a wrong TU. It fails closed and records the reason (E8).
  - Artifacts: receipts 10/27/40/51/60/70-72, `mutations/*`.
- **Tests** (clean; S2 optional). Checked:
  - Clean positive controls in 3 variants.
  - Library and consumer sensitivity is independent (M2/M3).
  - Compiler instrumentation is distinguished from runtime linkage (M1: runtime NEEDED, 0 references, diagnostics fail).
  - Partial per-sanitizer revert (M6a/b), plain library linked (M8), missing halt setting (M4), missing runtime (M5).
  - The clean policy catches reports (M7).
  - Child exit status and output are reported honestly (E5, E8).
  - Registration parity (E4); a direct 285 → 292 verdict (E3); no hidden reports in existing tests (E5); hosted proof on GCC 13.3 (E11).
  - Artifacts: `tests/sanitizer/*`, receipts 20-25/40, `mutations/*`, hosted.
- **Docs** (clean; S1 and S2 optional). The new text in `testing-and-ci.md` (lines 40, 54-90) and `architecture.md` (lines 191-198, 231-232) was checked against measured behavior:
  - build-type independence (E1, E2);
  - population and exclusions (E1, E2, E9);
  - ASan exit status 1 (E5, E8, receipt 91);
  - UBSan default recovery (M4), subject to the S1 nuance;
  - `halt_on_error` guidance (E6);
  - `LastTest.log` retention (E5);
  - the 7-case helper check and the checker's conditions (E4, E8);
  - the directory layout.

  Also checked: the legacy page (observation), README, and the `docs/INDEX.md` precedence rule.

## 5. Reviewer-owned ledger

| Lens | Round | Exact head | Open BLOCKER | Open MAJOR | Open MINOR | SUGGESTION | Coverage |
| --- | --- | --- | --- | --- | --- | --- | --- |
| Conformance | R231-1 | 76906b83eb54ac29040fe72fd910d34727188694 | 0 | 0 | 0 | S1 | clean |
| RTL (software/build architecture) | R231-1 | 76906b83eb54ac29040fe72fd910d34727188694 | 0 | 0 | 0 | — | clean |
| Robustness | R231-1 | 76906b83eb54ac29040fe72fd910d34727188694 | 0 | 0 | 0 | — | clean |
| Tests | R231-1 | 76906b83eb54ac29040fe72fd910d34727188694 | 0 | 0 | 0 | S2 | clean |
| Docs | R231-1 | 76906b83eb54ac29040fe72fd910d34727188694 | 0 | 0 | 0 | S1, S2 | clean |

This ledger covers R231 only. It says nothing about R232 (external), and A162's author evidence is not counted as review.

## 6. Validation limits

- **Clang and llvm-symbolizer** are not installed. There is no Clang compile or runtime evidence, and none is claimed.
- **CMake 3.21** was exercised only through the PyPI `cmake==3.21.4` wheel in `<SCRATCH>/venv-cmake321`, on this host with GCC 16.2.1. That is not a distro or native 3.21 install and not the hosted toolchain.
- **Hosted results** (GCC 13.3.0, ubuntu-24.04) were read from public job logs and not rerun.
  - Hosted CTest prints output only for failures, and the runner's `LastTest.log` is not retrievable. So recovered UBSan reports inside passing hosted tests are **unobserved**. Natively with GCC 16.2.1 there were none (E5).
  - Likewise, the hosted children's exit statuses for the passing diagnostics are not visible. Natively they are recorded (E5, E8).
- **Not rerun by R231** (A10 and hosted evidence stand for them):
  - the full explicit-Debug unit suite. R231 did run the Debug build with object inspection, the Debug probe (7/7) and M1 in Debug.
  - both BDD suites. They were deliberately not run to avoid fixed `/tmp` paths shared with other reviewers.
- **Not registered or not exercised:**
  - tshark is absent, so `stack_codec_tshark` is not registered; the 285 baseline assumes that.
  - The `@hardware` and `@verilator` scenarios were not exercised.
- **Configure-only checks:** Ninja Multi-Config, the custom build type and the embedded tests-ON consumer were only configured, not built or run.
- **Author-only claim:** the A162 aarch64 cross-compile claim was not reproduced.

## 7. Pending merge obligations (separate from the code verdict)

1. The independent external review R232 must also report positive with clean five-lens coverage. R231 cannot count for R232, and A162 cannot approve itself.
2. Manager A10 still owes:
   - live-candidate confirmation at merge time: main was still `deca300` during this review, so re-validate if it moves;
   - the merge itself;
   - post-merge containment. The published pre-merge history audit is only the baseline.
3. Publication should say that hosted UBSan recoveries in passing tests remain unobserved (section 6).
4. S1 and S2 are optional and do not gate the merge.
5. Clean-up of R231's generated artifacts (section 8) is at the manager's discretion.

## 8. Generated artifacts and checkout state

- **`<CLONE>`**:
  - HEAD, tree, index (SHA-256 `033a5457…74fd`), `ls-files -s` (SHA-256 `7382a7d7…3560`), tracked count (226) and `git status --porcelain=v2 --ignored` (empty) are identical before and after (`receipts/00-initial-state.txt`, `receipts/99-final-state.txt`).
  - Added by R231:
    - `external/googletest` at `0bdccf4`;
    - `external/rapidyaml` at `e65999d`, recursive: c4core `6c876ec`, c4core/cmake `b8e95ac`, debugbreak `328e4ab`, fast_float `d28a332`. All come from their public origins.
    - `submodule.external/{googletest,rapidyaml}.{active,url}` in `.git/config`, and `.git/modules/` (38 MB).
  - No commit, branch, stash or tracked-file change.
- **`<SCRATCH>`** (2.4 GB): verified base and candidate exports, mutation copies, build trees, consumer projects, `venv-cmake321` with its wheel, fake checker children, and the UBSan/ASan semantics programs.
- **`<OUT>/receipts`**: all logs, JSON, scripts and fetched public data. `receipts/MANIFEST.sha256` lists a SHA-256 for every receipt file.
- **Scope of actions:**
  - No public writes, pushes, merges, hardware, raw interfaces, Verilator DUT, Docker/act, global installs, product fixes or contact with other agents.
  - Public data was read only through `gh api`: the issue, PR, comments, reviews, check runs, workflow runs, job logs, the main branch and evidence blobs.
  - The only network downloads were the submodule clones and the CMake 3.21.4 wheel.

R231-1 FINISHED
