# [A162] Review-ready: issue 15 sanitizer instrumentation

Candidate `76906b83eb54ac29040fe72fd910d34727188694`, tree `a34065c39ea41aeb5daae9ecae3763f632f27484`, one commit over main `deca300c9eb4863fa13383b45ba6cb6fd0828671` on `15-instrument-sanitizer-targets`. Author evidence only, local and unpublished; no review verdict or hosted result is implied. Authority: the frozen decision https://github.com/kebag-logic/tsn-gen/issues/15#issuecomment-5772396945; TAKEN https://github.com/kebag-logic/tsn-gen/issues/15#issuecomment-5772572984.

## What to review

1. `cmake/CMakeGenTestingLibraries.cmake:16-38`: the sanitizer flag moves out of the four `$<CONFIG:...>` expressions and stays a PUBLIC option placed after them. The per-configuration `-O`/`-g` lists and `COMP_COVERAGE_FLAGS` placement are unchanged.
2. `CMakeLists.txt:63-66`: `tests/sanitizer` is added only when `ENABLE_PARSER_TESTS`.
3. `tests/sanitizer/`:
   - `sanitizer_probe_lib.cpp:13,18`: the library defects
   - `sanitizer_probe_test.cpp:29,34`: the consumer defects, with volatile inputs so no `-O` level folds them
   - `CMakeLists.txt:40-79`: clean controls with `FAIL_REGULAR_EXPRESSION`; diagnostic controls through the checker; `UBSAN_OPTIONS=halt_on_error=1` only on the two UBSan diagnostics
   - `check_sanitizer_child.cmake`: prints the child's exit status and output, and requires status 1, the report, "started" and no "completed"
4. `docs/low-level/testing-and-ci.md` (Sanitizer variants, new Sanitizer helper check) and `docs/low-level/architecture.md` (Sanitizer build matrix, layout): build-type independence and the exact ASan/UBSan exit semantics.

## Evidence map (this bundle)

| Question | Where |
| --- | --- |
| Exact generated compile/link/flags changes, 5 build types by 2 `BUILD_SHARED_LIBS` settings | `receipts/compare-base-vs-candidate.json`, `logs/021-compare-base-candidate.log`, raw captures in `receipts/configure/{base,cand}/` |
| Compiler instrumentation vs runtime linkage | `receipts/instrumentation-*.json` (per object `nm -u`, per artifact NEEDED / undefined dynamic symbols) |
| Unit and BDD gates, exits | `logs/010-*`, `031-*`, `040-*`, `043-*`, `062-*`, `072-074-*`; `receipts/results-summary.json` |
| Sanitizer reports in passing output | `receipts/scan-*.json` |
| Mutations and their patches | `receipts/mutations/M*.patch`, `receipts/mutations/05*/`, `logs/05*` |
| Consumers | `receipts/consumers/`, `logs/080-083-*` |
| Registration preserved | `receipts/ctest-registration-*.json`, `receipts/ctest-registration-compare.txt` |
| Second compiler (compile-only) | `receipts/cross/`, `logs/090-*`, `logs/091-*` |
| Every command with exit and log hash | `commands.jsonl`, `COMMANDS.md` |

## Fast independent reproduction (from a published checkout of the candidate)

```bash
git submodule update --init external/googletest
git submodule update --init --recursive external/rapidyaml
cmake -S . -B /tmp/i15-empty -DBUILD_SHARED_LIBS=ON -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
grep -c -- '-fsanitize=address' /tmp/i15-empty/compile_commands.json     # 31 (base: 0)
grep -c -- '-fsanitize=undefined' /tmp/i15-empty/compile_commands.json   # 31 (base: 0)
cmake --build /tmp/i15-empty -j8
ctest --test-dir /tmp/i15-empty --output-on-failure                       # 292/292
ctest --test-dir /tmp/i15-empty -R '^sanitizer_probe' -V                  # child exits and reports

# M1: restore the base helper; the 4 diagnostic controls must fail, clean ones pass
git show deca300c9eb4863fa13383b45ba6cb6fd0828671:cmake/CMakeGenTestingLibraries.cmake > cmake/CMakeGenTestingLibraries.cmake
cmake --build /tmp/i15-empty -j8 --target sanitizer_probe-test sanitizer_probe-test_ASAN sanitizer_probe-test_UBSAN
ctest --test-dir /tmp/i15-empty -R '^sanitizer_probe' --output-on-failure  # exit 8, 4 failed
git checkout -- cmake/CMakeGenTestingLibraries.cmake
```

## Known qualifications

A placeholder TU (`parser/src/db_proto_impl.cpp`) has no code, so its UBSan object holds no handler reference, as in base Debug. The following were not run by the author: Clang, CMake 3.21, hosted GCC 13.3, tshark, and the `@hardware`/`@verilator` scenarios. Legacy `docs/01_testing.md` keeps an inaccurate exit sentence; it is untouched legacy prose. See `HANDOFF.md` and `REMAINING-GATES.md`.
