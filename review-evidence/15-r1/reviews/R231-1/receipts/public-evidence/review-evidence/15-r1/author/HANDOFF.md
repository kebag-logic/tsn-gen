# [A162] Author handoff for tsn-gen issue 15

Head: `76906b83eb54ac29040fe72fd910d34727188694`, tree `a34065c39ea41aeb5daae9ecae3763f632f27484`. Base: main `deca300c9eb4863fa13383b45ba6cb6fd0828671`, tree `ac94d11ac446fcb5489a7845bae66921b86f5f01`. Branch: `15-instrument-sanitizer-targets`. Checkout: `$VALIDATION_STORAGE/lanes/tsngen15-sanitizer-targets`. One local author commit (`Instrument sanitizer variants and their test TUs under every build type`, no trailers). The worktree is clean, including ignored files. Nothing is pushed and no PR exists.

Public records:
- Issue: https://github.com/kebag-logic/tsn-gen/issues/15
- Frozen decision: https://github.com/kebag-logic/tsn-gen/issues/15#issuecomment-5772396945
- A162 TAKEN: https://github.com/kebag-logic/tsn-gen/issues/15#issuecomment-5772572984
- A162 author candidate and material assumptions: https://github.com/kebag-logic/tsn-gen/issues/15#issuecomment-5772855876 (posted bodies of both comments match `TAKEN.md` and `public/candidate-comment.md` apart from GitHub's stripped final newline)

## Change

| File | Change |
| --- | --- |
| `cmake/CMakeGenTestingLibraries.cmake` | `COMP_ASAN_FLAGS` / `COMP_UBSAN_FLAGS` leave the four `$<CONFIG:...>` expressions. They stay in the same PUBLIC `target_compile_options` call, placed after the per-configuration `-O`/`-g` expressions. `COMP_COVERAGE_FLAGS` keeps its per-configuration placement. |
| `CMakeLists.txt` | `add_subdirectory(tests/sanitizer)` only when `ENABLE_PARSER_TESTS` |
| `tests/sanitizer/` (new) | Fixture library `sanitizer_probe` built through `generate_test_libs`; probe `sanitizer_probe-test{,_ASAN,_UBSAN}` built through `target_link_testlibs`; `check_sanitizer_child.cmake` checker; 7 CTest cases |
| `docs/low-level/testing-and-ci.md`, `docs/low-level/architecture.md` | Build-type independence, instrumented and uninstrumented targets, actual ASan and UBSan exit semantics, and the helper check. The removed sentences said any sanitizer report exits non-zero. |

`target_link_testlibs`, `gtest_discover_test_wtestlibs`, every component CMakeLists, all sources, protocol YAML, expected frames and assertions are unchanged. No global build type, no global sanitizer or recovery option, and no sanitized protocol_logic or third-party target exists. The new checker matches `.gitignore`'s `*.cmake` rule, so it was added with `git add -f`, like the tracked `cmake/*.cmake` modules.

## Acceptance mapping (frozen decision)

| Decision item | Evidence |
| --- | --- |
| Variants and consuming test TUs instrumented independently of build type, including empty | Generated commands in all 10 configurations (5 build types by `BUILD_SHARED_LIBS` unset/ON): commands 003, 020, 021/084. The candidate has 31 ASan-flagged and 31 UBSan-flagged compile commands in every configuration; base has 0/0 in the empty type. A custom `Profile` type (configure-only) gives the same result: base 0/0, candidate 31/31 (085) |
| Keep per-configuration `-O`/`-g` choices | In Debug, Release, MinSizeRel and RelWithDebInfo, all 125 existing compile commands, 41 link.txt and 41 flags.make files are byte-identical to base, for both `BUILD_SHARED_LIBS` settings |
| Empty type: exact change only | 67 of 125 compile commands identical; the 29 + 29 variant objects each gain exactly one matching `-fsanitize` with the remaining token order unchanged; all 41 link commands identical |
| Test registration unchanged | The first 285 candidate CTest registrations equal base in order, including commands and properties; 7 probe cases are appended (`receipts/ctest-registration-compare.txt`) |
| Ordinary consumers, embedded/tests-off unchanged | Tests-off top level and embedded `add_subdirectory` (empty, Debug) build and run with exit 0 for base and candidate. No `-fsanitize` appears in any of their commands, and their generated files are identical between base and candidate (080 to 083) |
| Independent normal and diagnostic controls through the actual helpers, which detect removal of instrumentation | Probe controls pass 7/7 in all five build types. Mutations M1 to M5 fail exactly the expected controls (see below) |
| Record compiler instrumentation and runtime linkage distinctly | Object references (`nm -u`) and NEEDED entries (`readelf -d`) for base and candidate builds (034, 042, 045); M5 link failure without the runtime |
| UBSan fatal setting explicit; actual child exits kept | `UBSAN_OPTIONS=halt_on_error=1` only on the two UBSan diagnostic controls (CTest ENVIRONMENT property). The checker prints the child's exit status and output and requires exactly 1; M4 shows the default recovery (child exit 0 after the report) |
| Default and explicit Debug native unit tests | `tests/run-tests.sh` exit 0: default 292/292 (base 285/285), Debug 292/292 (base 285/285) |
| Both documented BDD suites | Exit 0 with stock tag exclusions, base and candidate identical |
| No protocol value, frame or assertion change | Source diff touches no protocol, test source or assertion outside the new `tests/sanitizer/` |

## Author validation

All commands ran on CPUs 96-103 (8 CPUs, memory.max 12 GiB). Builds were sequential, each capped at 8 jobs (`make -j8` or `cmake --build -j8`; the literal `setup.sh` replay ran with `nproc` = 8). CTest ran serially. Tools: GCC 16.2.1, GNU ld 2.47, CMake/CTest 4.4.3, GNU Make 4.4.1, Python 3.14.7, behave 1.3.3 in a disposable venv outside the checkout (`receipts/tools.txt`). Only `external/googletest` and `external/rapidyaml` (recursive) were initialized, from their public origins, without object alternates. The checkout's own object store was provisioned with an alternates entry pointing at `$VALIDATION_STORAGE/lanes/tsngen15-readiness/.git/objects`. That entry predates this task (`logs/000-initial-state.txt`) and A162 did not change it. Builds used clean `git archive` exports of the exact commits. Every exported blob matches its commit: 219 for the candidate and replay exports, 214 for base (`receipts/export-vs-commit.txt`).

| Gate | Commands | Result |
| --- | --- | --- |
| Base default (setup.sh configuration: empty type, `BUILD_SHARED_LIBS=ON`, prep) | 010 | configure/build/prep 0; `tests/run-tests.sh` 0, 285/285 |
| Candidate default, same configuration | 031 | configure/build/prep 0, 0 warnings; `tests/run-tests.sh` 0, 292/292 |
| Base / candidate explicit Debug (documented flow) | 040 | 0, 285/285 / 0, 292/292; 0 warnings |
| Candidate Release, MinSizeRel, RelWithDebInfo | 043 | 0, 292/292 each; 0 warnings (Release adds `-Werror -pedantic`) |
| Literal workflow replay on a fresh export: `./setup.sh d`, `./tests/run-tests.sh`, `./tests/run-tests-behave.sh` | 070 to 074 | 0 (292/292), 0 (292/292), 0 (both suites pass) |
| Both BDD suites, `--tags=-hardware --tags=-verilator`, base and candidate | 062 | 0 and 0 each. AECP: 8 features, 96 scenarios, 395 steps passed; 2 scenarios / 8 steps skipped (`@hardware` raw interface, `@verilator` DUT). Stack: 4 features, 115 scenarios, 563 steps passed |
| Sanitizer reports in any test output (LastTest.log), all builds | 032, 041, 044 | none outside the 4 probe diagnostic controls, which carry their expected reports |
| Consumer controls | 080 to 083 | all exits 0; zero sanitizer flags; generated files identical to base |
| Second compiler, compile-only | 090, 091 | see below |

The fixed `/tmp/*.pcap` paths written by the BDD features did not exist before the first suite run (`logs/062-bdd-cand-default-tmp-before.txt`).

## Instrumented population and exclusions

Every build type of the candidate, including empty, compiles with the matching sanitizer flag:
- `protocol_parser_asan` / `_ubsan`: the 11 `PROTO_PARSER_SRCS`
- `traffic_gen_asan` / `_ubsan`: the 10 `TRAFFIC_GEN_SRCS`
- the `_ASAN` / `_UBSAN` test TUs `protocol_parser_test.cpp`, `object_serializer_test.cpp`, `logic_registry_test.cpp`, `stack_builder_test.cpp`, `validator_test.cpp`, `traffic_gen_test.cpp`, `adp_fuzz_test.cpp`, `ptp_flags_test.cpp`
- probe: `sanitizer_probe_lib.cpp` (variants) and `sanitizer_probe_test.cpp` (`_ASAN` / `_UBSAN`)

That is 31 objects per sanitizer. Header code from rapidyaml and googletest is instrumented only where these TUs compile it.

Not instrumented, unchanged from base: the plain libraries and plain test executables, `gtest_parser-test`, `protocol_logic` and its tests, `packet_gen`, and the vendored rapidyaml and googletest targets (ryml, c4core, gtest, gtest_main, gmock, gmock_main).

Object evidence, candidate, every build type: all 31 ASan-flagged objects reference `__asan_*`; 30 of 31 UBSan-flagged objects reference `__ubsan_handle_*`. The exception is `parser/src/db_proto_impl.cpp`, a placeholder TU with no code for UBSan to check; base Debug shows the same exception. In the base empty type, 0 of 125 objects hold any sanitizer reference, while 16 `_ASAN`/`_UBSAN` executables list `libasan.so.8` or `libubsan.so.1` as NEEDED: runtime linkage without compiler instrumentation.

Runtime linkage is unchanged: the 16 (plus 2 probe) sanitized executables are linked with `-fsanitize` and NEED the runtime. The variant shared libraries are linked without it in both trees and leave their sanitizer symbols undefined for the executable's runtime. For example, candidate `libprotocol_parser_asan.so` has 33 undefined `__asan_*` dynamic symbols, where base empty has 0.

## Regression and mutation receipts

Probe, CTest cases (`tests/sanitizer/CMakeLists.txt`):
- clean: `sanitizer_probe-test.clean`, `_ASAN.clean`, `_UBSAN.clean`. Each executable runs directly; exit 0 is required and `FAIL_REGULAR_EXPRESSION "Sanitizer|runtime error"` rejects any report.
- diagnostic: `_ASAN.library-heap-overflow`, `_ASAN.consumer-heap-overflow`, `_UBSAN.library-signed-overflow`, `_UBSAN.consumer-signed-overflow`. Each defect lives in one TU (the fixture library or the consuming probe TU) and needs that TU's instrumentation to be seen. The checker requires child exit status 1, the report, the "started" line and no "completed" line.

Observed candidate diagnostics (empty type): ASan heap-buffer-overflow with frame #0 in `sanitizer_probe::libraryRead` in `libsanitizer_probe_asan.so` for the library case; UBSan `sanitizer_probe_lib.cpp:18:18` and `sanitizer_probe_test.cpp:34:18` signed overflow; child exit 1 each.

| Mutation (patch in `receipts/mutations/`) | Build type | Result |
| --- | --- | --- |
| M0 none | empty | 7/7 pass (050) |
| M1 base helper | empty | CTest 8; the 4 diagnostics fail: children exit 0, no report, run past the fault; clean 3/3 pass (051) |
| M1 base helper | Debug | 7/7 pass, matching the defect's dependence on build type (052) |
| M2 sanitizer flag PRIVATE (library only) | empty | CTest 8; only the 2 consumer diagnostics fail (053) |
| M3 sanitizer flag INTERFACE (consumers only) | empty | CTest 8; only the 2 library diagnostics fail (054) |
| M4 no `UBSAN_OPTIONS=halt_on_error=1` | empty | CTest 8; only the 2 UBSan diagnostics fail: report printed, child continues and exits 0 (055) |
| M5 no link-time `-fsanitize` | empty | `_ASAN` and `_UBSAN` probe links fail on undefined `__asan_*` (23) / `__ubsan_*` (24) references (056, 057) |

## Second compiler

Clang, clang++ and llvm-symbolizer are not installed. The only other C++ compiler is `aarch64-linux-gnu-g++` 16.1.0, whose toolchain ships no aarch64 `libasan`/`libubsan`, and no emulator is present. Its control is therefore compile-only. With the candidate, all 28 inspected objects carry the flag (probe library and TU, `protocol_parser_{asan,ubsan}`, `ptp_flags-test_{ASAN,UBSAN}`), and 27 hold the matching references (the exception is again `db_proto_impl.cpp` for UBSan). With the base helper, none carries the flag or a reference. Variant shared libraries link (exit 0); the sanitized executable link fails with `cannot find -lasan` (exit 2). No runtime or behavior claim is made for this compiler.

## Material assumptions

1. ASan diagnostic controls rely on ASan's default fatal behavior and default exit status 1. The only declared behavior setting is `UBSAN_OPTIONS=halt_on_error=1`, on the two UBSan diagnostic controls; every other test keeps the default UBSan recovery.
2. For a diagnostic control the checker's exit is the CTest verdict. The child's own exit status and output are printed unchanged, and the status must be exactly 1.
3. The 7 probe registrations are the requested regression; no existing registration changes.
4. `COMP_COVERAGE_FLAGS` (empty at the root) is not sanitizer instrumentation and keeps its per-configuration placement.
5. The variant libraries keep their existing link (no `-fsanitize`), since the decision covers compilation; runtime linkage stays with `target_link_testlibs`.
6. Legacy `docs/01_testing.md` still says a sanitizer error exits non-zero. It is untouched legacy prose, which `docs/INDEX.md` places below the tier pages.

## Not covered by this author lane

Hosted workflows (the issue 14 author evidence records GNU 13.3.0 on the hosted Ubuntu 24.04 image); CMake 3.21 (the new CMake uses only commands and properties available there, but this was not executed); Clang; tshark (absent, `stack_codec_tshark` not registered); `@hardware` / `@verilator` scenarios (excluded by the stock tags); hardware acceptance. Hosted CTest output shows only failing tests, so a recovered UBSan report in a passing hosted test would not be visible there. Exact remaining gates are listed in `REMAINING-GATES.md`.

## Deviations and failed attempts

- 033: the first instrumentation run expected `compile_commands.json`, which the setup.sh configuration does not export (exit 1, no measurement). 034 reads each target's `flags.make` instead; the configuration was not changed.
- 082: the first consumer comparison did not neutralize the tsn-gen root passed as `TSN_GEN_SOURCE_DIR`, so path differences showed as "other". 083 neutralizes it and finds all files identical. 084 re-runs the matrix comparison with the updated script and reproduces 021 exactly.
- The probe was developed with informal builds under `<scratch>/build/dev-*` before the commit. They are not evidence.

## Layout

`TAKEN.md` (posted text), `PR-BODY.md`, `REVIEW-READY.md`, `REMAINING-GATES.md`, `COMMANDS.md`, `commands.jsonl` (every command: argv, cwd, UTC times, exact exit, log SHA-256), `logs/`, `receipts/` (configure captures, comparisons, instrumentation, scans, mutations, cross, consumers, registration, tools, results-summary.json), `scripts/`, `source.patch`, `public/`, `MANIFEST.json`. Scratch builds: `$VALIDATION_STORAGE/tmp/a162-tsngen15` (`scratch-path.txt`).
