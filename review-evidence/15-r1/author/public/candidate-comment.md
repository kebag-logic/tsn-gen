[A162] Author candidate for issue 15 (local, unpublished)

Candidate `76906b83eb54ac29040fe72fd910d34727188694`, tree `a34065c39ea41aeb5daae9ecae3763f632f27484`: one commit on `15-instrument-sanitizer-targets` over main `deca300c9eb4863fa13383b45ba6cb6fd0828671`. Source is frozen for review. This is author evidence only. It is not pushed, has no PR, and carries no review verdict or hosted result. Scope and validation follow the [TAKEN record](https://github.com/kebag-logic/tsn-gen/issues/15#issuecomment-5772572984).

**Change**
- `cmake/CMakeGenTestingLibraries.cmake`: `COMP_ASAN_FLAGS`/`COMP_UBSAN_FLAGS` leave the four `$<CONFIG:...>` expressions. They stay in the same PUBLIC `target_compile_options` call, after those expressions. The `-O`/`-g` choices and the `COMP_COVERAGE_FLAGS` placement are unchanged.
- `tests/sanitizer/` (new; the root adds it only with `ENABLE_PARSER_TESTS`): a fixture library built by `generate_test_libs`, a probe built by `target_link_testlibs`, and 7 CTest cases.
- `docs/low-level/testing-and-ci.md`, `docs/low-level/architecture.md`: build-type independence, the instrumented population, and the actual exit semantics. The removed sentences claimed that every sanitizer report exits non-zero.

**Generated commands** (configure-only; Unix Makefiles; `BUILD_SHARED_LIBS` unset and `ON`)
- Debug, Release, MinSizeRel, RelWithDebInfo: all 125 existing compile commands, 41 `link.txt` and 41 `flags.make` files are byte-identical to base.
- Empty type: 67 compile commands are identical. The other 29 + 29 each gain exactly the matching `-fsanitize`, with token order otherwise unchanged: `protocol_parser_{asan,ubsan}` 11 each, `traffic_gen_{asan,ubsan}` 10 each, and the 8 `_ASAN` and 8 `_UBSAN` test TUs. Every link command is identical.
- The probe adds 6 compile and 6 link entries in every configuration. A custom `Profile` type gives the same pattern: base 0/0 flagged, candidate 31/31.

**Compiler instrumentation and runtime linkage** (built with the setup.sh configuration)
- Base, empty type: 0 of 125 objects reference any sanitizer entry point, yet 16 test executables record `libasan.so.8`/`libubsan.so.1` as NEEDED.
- Candidate, in each of the five build types: all 31 ASan-flagged objects reference `__asan_*`, and 30 of 31 UBSan-flagged objects reference `__ubsan_handle_*`. The exception is `parser/src/db_proto_impl.cpp`, a placeholder TU with no code to check; it is the same in base Debug.
- Variant shared libraries stay linked without `-fsanitize` and leave their sanitizer symbols to the executable's runtime.

**Behavior** (GCC 16.2.1, CMake 4.4.3, 8 jobs, CTest serial)

| Gate | Result |
| --- | --- |
| setup.sh-equivalent default (empty, `BUILD_SHARED_LIBS=ON`, prep), then direct `tests/run-tests.sh` | base exit 0, 285/285; candidate exit 0, 292/292 |
| Documented explicit Debug, then direct `tests/run-tests.sh` | base 0, 285/285; candidate 0, 292/292 |
| Candidate Release, MinSizeRel, RelWithDebInfo | 0, 292/292 each, no warnings |
| Literal replay on a fresh export: `./setup.sh d`; `./tests/run-tests.sh`; `./tests/run-tests-behave.sh` | 0; 0 (292/292); 0 |
| Both BDD suites, stock `--tags=-hardware --tags=-verilator` | AECP 96 passed, 2 skipped; stack 115 passed; base identical |
| Tests-off top level; embedded `add_subdirectory` (empty, Debug) | build and run 0; no `-fsanitize` anywhere; generated files identical to base |
| Registration | the first 285 candidate registrations equal base in order; 7 probe cases appended |

No existing test's output contains a sanitizer report in any of these runs; only the 4 probe diagnostic controls do.

**Mutation receipts** (probe only, empty type unless noted)
- Base helper restored: the 4 diagnostic controls fail. Their children exit 0 with no report and continue past the fault; the clean controls pass. In Debug the same mutation passes 7/7, matching the defect's dependence on build type.
- Library-only (PRIVATE) instrumentation fails only the 2 consumer controls. Consumer-only (INTERFACE) fails only the 2 library controls.
- Without `UBSAN_OPTIONS=halt_on_error=1`, only the 2 UBSan controls fail: each child prints its `runtime error`, continues and exits 0, so a stderr match alone is not accepted.
- Without the link-time `-fsanitize`, the `_ASAN` and `_UBSAN` probe links fail on undefined `__asan_*`/`__ubsan_*` references.

**Second compiler:** Clang is not installed. With aarch64-linux-gnu GCC 16.1.0 (compile-only: no target libasan/libubsan and no emulator), the candidate gives all 28 inspected probe, `protocol_parser_{asan,ubsan}` and `ptp_flags-test_{ASAN,UBSAN}` objects the flag, and 27 carry references (the same UBSan placeholder exception). With the base helper, none has either. Sanitized executable links fail with `cannot find -lasan`, so no runtime claim is made.

**Material assumptions**
1. The ASan diagnostic controls rely on ASan's default fatal behavior and default exit status 1. The only declared behavior setting is `UBSAN_OPTIONS=halt_on_error=1`, on the two UBSan diagnostic controls; every other test keeps UBSan's default recovery.
2. For a diagnostic control, the checker's exit is the CTest verdict. The child's own exit status and output are printed unchanged, and the status must be exactly 1.
3. The 7 probe registrations are the requested regression; no existing registration changes.
4. `COMP_COVERAGE_FLAGS` is not sanitizer instrumentation and keeps its per-configuration placement.
5. Variant libraries keep their existing link, without `-fsanitize`; runtime linkage stays with `target_link_testlibs`.
6. `.gitignore` matches `*.cmake`, so the new checker is force-added, like the existing `cmake/*.cmake` modules.
7. Legacy `docs/01_testing.md` still says a sanitizer error exits non-zero. It is untouched legacy prose, which `docs/INDEX.md` places below the tier pages.

No out-of-scope product defect surfaced, and there are no conflicts or dependencies to report.

**Not covered:** hosted workflows (GCC 13.3 per the issue 14 evidence), CMake 3.21, Clang runtime behavior, tshark (absent), and the `@hardware`/`@verilator` scenarios. In hosted CI a recovered UBSan report in a passing test is not printed by `--output-on-failure`. Publication, independent reviews, hosted runs and merge stay with the manager.
