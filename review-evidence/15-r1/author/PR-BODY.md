Instrument sanitizer variants and their test TUs under every build type

Closes #15

`generate_test_libs` added `-fsanitize=address` / `-fsanitize=undefined` only inside the Debug, Release, MinSizeRel and RelWithDebInfo generator expressions. The empty default build type, which `setup.sh` and CI use, therefore compiled the `_asan`/`_ubsan` libraries and the `_ASAN`/`_UBSAN` test TUs without instrumentation while their executables still linked the sanitizer runtimes. The flag now sits outside those expressions as a PUBLIC compile option, so each variant and every test TU linking it are instrumented under any build type. Unchanged: the per-configuration `-O`/`-g` choices, `target_link_testlibs` linkage, plain and embedded/tests-off consumers, source populations, existing test registrations, protocol values and frame assertions.

`tests/sanitizer/` adds a regression built through the real helpers: a fixture library from `generate_test_libs` and a probe from `target_link_testlibs`. Three clean controls must exit 0 without a report. Four diagnostic controls each place one defect, a heap overflow (ASan) or a signed overflow (UBSan), either in the fixture library or in the consuming probe TU. A checker prints each child's actual exit status and output and requires status 1, the report and no progress past the fault. UBSan recovers by default, so only its two controls set `UBSAN_OPTIONS=halt_on_error=1`. The testing and architecture pages now describe these exit semantics accurately.

Author validation (GCC 16.2.1, CMake 4.4.3):
- **Generated commands:** in Debug, Release, MinSizeRel and RelWithDebInfo, every existing compile, link and flags file is byte-identical to main. In the empty type, exactly the 29 + 29 variant objects gain their matching flag, and no link command changes.
- **Unit runs:** default and explicit Debug pass 292/292 (285 existing plus 7 new), and so do Release, MinSizeRel and RelWithDebInfo.
- **BDD and consumers:** both BDD suites and the tests-off/embedded consumers match main. No existing test prints a sanitizer report.
- **Mutations:** restoring the old helper fails exactly the four diagnostic controls in the empty type. Library-only and consumer-only instrumentation each fail only their own two controls. Removing the UBSan fatal setting exposes the default recovery.

[Author evidence and material assumptions](https://github.com/kebag-logic/tsn-gen/issues/15#issuecomment-5772855876).

Not covered by the author: hosted CI, CMake 3.21, Clang, tshark and the `@hardware`/`@verilator` scenarios. No hardware acceptance is claimed. Executor A162; reviewers R231/R232.

🤖 Generated with [Claude Code](https://claude.com/claude-code)
