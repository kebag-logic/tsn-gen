[A162] TAKEN

Branch `15-instrument-sanitizer-targets` at main `deca300c9eb4863fa13383b45ba6cb6fd0828671`, isolated checkout, sole author. Authority: the frozen [readiness decision](https://github.com/kebag-logic/tsn-gen/issues/15#issuecomment-5772396945). R231/R232 review afterwards. Publication, hosted runs, containment and merge stay with the manager; this lane does not push or open a PR.

**Scope**

- `generate_test_libs` keeps each configuration's `-O`/`-g` choice and moves only `COMP_ASAN_FLAGS`/`COMP_UBSAN_FLAGS` outside the configuration expressions. The `_asan`/`_ubsan` variants, and every test translation unit consuming their PUBLIC compile requirements, are then instrumented for any build type, including the empty default.
- Unchanged: `target_link_testlibs` linkage, plain targets, embedded/tests-off defaults, source populations, existing test registration, protocol values, frames and assertions. No global build type, no global sanitizer or recovery flag, no newly sanitized protocol_logic or third-party target. `COMP_COVERAGE_FLAGS` keeps its per-configuration placement.
- Regression through the real helpers, in a new `tests/sanitizer/` added from the root under `ENABLE_PARSER_TESTS`: a small fixture library from `generate_test_libs` and a probe from `target_link_testlibs`, registered with CTest. Passing controls (plain, ASAN, UBSAN) must exit 0 with no sanitizer report. Diagnostic controls trigger a heap overflow (ASAN) and a signed overflow (UBSAN), separately in the library TU and in the consuming test TU. A checker records the child's actual exit status and output, and requires exit 1, the expected report and no continuation past the fault. Only the UBSAN diagnostic controls set `UBSAN_OPTIONS=halt_on_error=1`, recorded in their test properties; ASan's default is already fatal.
- Touched docs (`docs/low-level/testing-and-ci.md`, `docs/low-level/architecture.md`) will state build-type independence and the actual exit semantics: an ASan report stops the process with exit 1 by default, while UBSan recovers by default, so its report alone does not fail a test.

**Focused validation** (GCC 16.2.1, CMake 4.4.3, separate out-of-tree builds, at most 8 jobs; only `external/rapidyaml` recursively and `external/googletest` initialized)

1. Complete `compile_commands.json` and every `link.txt`, base and candidate, for empty, Debug, Release, MinSizeRel and RelWithDebInfo, with `BUILD_SHARED_LIBS` unset and `ON`. Acceptance: existing commands byte-identical, except that the empty type gains exactly the matching `-fsanitize` on `_asan`/`_ubsan` library and test objects.
2. Compiler instrumentation (sanitizer references in objects) and runtime linkage (link flags, NEEDED entries) recorded separately, base and candidate.
3. Probe controls in all five build types. Mutation receipts: the base helper (instrumentation removed) must fail the diagnostic controls in the empty type; library-only instrumentation must fail only the test-TU controls; the UBSAN control without the fatal setting must show default recovery (child exit 0).
4. setup.sh-equivalent default build (empty type, `BUILD_SHARED_LIBS=ON`, `prep-tests.sh`) and an explicit Debug build, each followed by a direct `tests/run-tests.sh` with its exit kept; both BDD suites with the stock `--tags=-hardware --tags=-verilator`; all CTest logs scanned for sanitizer reports.
5. Tests-off top-level and embedded `add_subdirectory` consumers build and run with no `-fsanitize` in any command.
6. Second compiler: Clang is not installed. Only an aarch64 GCC 16.1.0 cross compiler exists, without target sanitizer runtimes or an emulator, so that control is compile-only and labelled so.

**Blockers:** none known. Any product diagnostic newly exposed in the default build will be reported here as a conflict/dependency, not suppressed. Hosted jobs, CMake 3.21 and Clang runtime behavior are outside this author lane.
