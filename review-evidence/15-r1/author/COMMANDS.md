# Commands (A162, tsn-gen issue 15)

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
| 001-submodule-googletest | 0 | 2026-09-22T07:12:56Z | Initialize external/googletest only (read by external/CMakeLists.txt when tests are on) | `logs/001-submodule-googletest.log` |
| 002-submodule-rapidyaml | 0 | 2026-09-22T07:12:58Z | Initialize external/rapidyaml recursively (documented by external/CMakeLists.txt) | `logs/002-submodule-rapidyaml.log` |
| 003-configure-matrix-base | 0 | 2026-09-22T07:13:57Z | Configure-only matrix of unmodified base deca300 (5 build types x BUILD_SHARED_LIBS unset/ON) | `logs/003-configure-matrix-base.log` |
| 004-export-base | 0 | 2026-09-22T07:14:25Z | Export clean base deca300 tree plus initialized submodules to scratch | `logs/004-export-base.log` |
| 010-base-default-configure | 0 | 2026-09-22T07:14:49Z | setup.sh configure step: cmake <src> -DBUILD_SHARED_LIBS=ON  | `logs/010-base-default-configure.log` |
| 010-base-default-build | 0 | 2026-09-22T07:14:49Z | setup.sh build step, capped: make -j8 | `logs/010-base-default-build.log` |
| 010-base-default-prep | 0 | 2026-09-22T07:15:03Z | setup.sh prepare step: parser/tests/prep-tests.sh | `logs/010-base-default-prep.log` |
| 010-base-default-unit | 0 | 2026-09-22T07:15:04Z | Unit gate: tests/run-tests.sh <build> invoked directly | `logs/010-base-default-unit.log` |
| 011-configure-matrix-helper-only | 0 | 2026-09-22T07:15:56Z | Configure-only matrix with only the helper change applied (before the regression was added) | `logs/011-configure-matrix-helper-only.log` |
| 020-configure-matrix-candidate | 0 | 2026-09-22T07:21:47Z | Configure-only matrix of candidate 76906b8 (5 build types x BUILD_SHARED_LIBS unset/ON) | `logs/020-configure-matrix-candidate.log` |
| 021-compare-base-candidate | 0 | 2026-09-22T07:21:57Z | Compare base and candidate generated compile/link/flags files per configuration | `logs/021-compare-base-candidate.log` |
| 030-export-candidate | 0 | 2026-09-22T07:22:07Z | Export clean candidate 76906b8 tree plus initialized submodules to scratch | `logs/030-export-candidate.log` |
| 031-cand-default-configure | 0 | 2026-09-22T07:22:07Z | setup.sh configure step: cmake <src> -DBUILD_SHARED_LIBS=ON  | `logs/031-cand-default-configure.log` |
| 031-cand-default-build | 0 | 2026-09-22T07:22:08Z | setup.sh build step, capped: make -j8 | `logs/031-cand-default-build.log` |
| 031-cand-default-prep | 0 | 2026-09-22T07:22:23Z | setup.sh prepare step: parser/tests/prep-tests.sh | `logs/031-cand-default-prep.log` |
| 031-cand-default-unit | 0 | 2026-09-22T07:22:23Z | Unit gate: tests/run-tests.sh <build> invoked directly | `logs/031-cand-default-unit.log` |
| 032-scan-base-default | 0 | 2026-09-22T07:23:01Z | Scan base-default LastTest.log for sanitizer reports | `logs/032-scan-base-default.log` |
| 032-scan-cand-default | 0 | 2026-09-22T07:23:01Z | Scan cand-default LastTest.log for sanitizer reports | `logs/032-scan-cand-default.log` |
| 033-instrumentation-base-default | 1 | 2026-09-22T07:23:28Z | Object sanitizer references and runtime NEEDED entries of base-default | `logs/033-instrumentation-base-default.log` |
| 033-instrumentation-cand-default | 1 | 2026-09-22T07:23:28Z | Object sanitizer references and runtime NEEDED entries of cand-default | `logs/033-instrumentation-cand-default.log` |
| 034-instrumentation-base-default | 0 | 2026-09-22T07:23:53Z | Object sanitizer references and runtime NEEDED entries of base-default (flags.make based) | `logs/034-instrumentation-base-default.log` |
| 034-instrumentation-cand-default | 0 | 2026-09-22T07:23:55Z | Object sanitizer references and runtime NEEDED entries of cand-default (flags.make based) | `logs/034-instrumentation-cand-default.log` |
| 040-base-debug-configure | 0 | 2026-09-22T07:24:22Z | Documented configure: cmake -S <src> -B <build> -DCMAKE_BUILD_TYPE=Debug | `logs/040-base-debug-configure.log` |
| 040-base-debug-build | 0 | 2026-09-22T07:24:23Z | Documented build, capped: cmake --build <build> -j8 | `logs/040-base-debug-build.log` |
| 040-base-debug-unit | 0 | 2026-09-22T07:24:41Z | Unit gate: tests/run-tests.sh <build> invoked directly | `logs/040-base-debug-unit.log` |
| 040-cand-debug-configure | 0 | 2026-09-22T07:24:44Z | Documented configure: cmake -S <src> -B <build> -DCMAKE_BUILD_TYPE=Debug | `logs/040-cand-debug-configure.log` |
| 040-cand-debug-build | 0 | 2026-09-22T07:24:45Z | Documented build, capped: cmake --build <build> -j8 | `logs/040-cand-debug-build.log` |
| 040-cand-debug-unit | 0 | 2026-09-22T07:25:05Z | Unit gate: tests/run-tests.sh <build> invoked directly | `logs/040-cand-debug-unit.log` |
| 041-scan-base-debug | 0 | 2026-09-22T07:25:16Z | Scan base-debug LastTest.log for sanitizer reports | `logs/041-scan-base-debug.log` |
| 042-instrumentation-base-debug | 0 | 2026-09-22T07:25:16Z | Object sanitizer references and runtime NEEDED entries of base-debug | `logs/042-instrumentation-base-debug.log` |
| 041-scan-cand-debug | 0 | 2026-09-22T07:25:19Z | Scan cand-debug LastTest.log for sanitizer reports | `logs/041-scan-cand-debug.log` |
| 042-instrumentation-cand-debug | 0 | 2026-09-22T07:25:19Z | Object sanitizer references and runtime NEEDED entries of cand-debug | `logs/042-instrumentation-cand-debug.log` |
| 043-cand-release-configure | 0 | 2026-09-22T07:25:28Z | Documented configure: cmake -S <src> -B <build> -DCMAKE_BUILD_TYPE=Release | `logs/043-cand-release-configure.log` |
| 043-cand-release-build | 0 | 2026-09-22T07:25:29Z | Documented build, capped: cmake --build <build> -j8 | `logs/043-cand-release-build.log` |
| 043-cand-release-unit | 0 | 2026-09-22T07:26:00Z | Unit gate: tests/run-tests.sh <build> invoked directly | `logs/043-cand-release-unit.log` |
| 043-cand-minsizerel-configure | 0 | 2026-09-22T07:26:02Z | Documented configure: cmake -S <src> -B <build> -DCMAKE_BUILD_TYPE=MinSizeRel | `logs/043-cand-minsizerel-configure.log` |
| 043-cand-minsizerel-build | 0 | 2026-09-22T07:26:03Z | Documented build, capped: cmake --build <build> -j8 | `logs/043-cand-minsizerel-build.log` |
| 043-cand-minsizerel-unit | 0 | 2026-09-22T07:26:22Z | Unit gate: tests/run-tests.sh <build> invoked directly | `logs/043-cand-minsizerel-unit.log` |
| 043-cand-relwithdebinfo-configure | 0 | 2026-09-22T07:26:24Z | Documented configure: cmake -S <src> -B <build> -DCMAKE_BUILD_TYPE=RelWithDebInfo | `logs/043-cand-relwithdebinfo-configure.log` |
| 043-cand-relwithdebinfo-build | 0 | 2026-09-22T07:26:25Z | Documented build, capped: cmake --build <build> -j8 | `logs/043-cand-relwithdebinfo-build.log` |
| 043-cand-relwithdebinfo-unit | 0 | 2026-09-22T07:26:52Z | Unit gate: tests/run-tests.sh <build> invoked directly | `logs/043-cand-relwithdebinfo-unit.log` |
| 044-scan-cand-release | 0 | 2026-09-22T07:27:01Z | Scan cand-release LastTest.log for sanitizer reports | `logs/044-scan-cand-release.log` |
| 045-instrumentation-cand-release | 0 | 2026-09-22T07:27:01Z | Object sanitizer references and runtime NEEDED entries of cand-release | `logs/045-instrumentation-cand-release.log` |
| 044-scan-cand-minsizerel | 0 | 2026-09-22T07:27:03Z | Scan cand-minsizerel LastTest.log for sanitizer reports | `logs/044-scan-cand-minsizerel.log` |
| 045-instrumentation-cand-minsizerel | 0 | 2026-09-22T07:27:03Z | Object sanitizer references and runtime NEEDED entries of cand-minsizerel | `logs/045-instrumentation-cand-minsizerel.log` |
| 044-scan-cand-relwithdebinfo | 0 | 2026-09-22T07:27:06Z | Scan cand-relwithdebinfo LastTest.log for sanitizer reports | `logs/044-scan-cand-relwithdebinfo.log` |
| 045-instrumentation-cand-relwithdebinfo | 0 | 2026-09-22T07:27:06Z | Object sanitizer references and runtime NEEDED entries of cand-relwithdebinfo | `logs/045-instrumentation-cand-relwithdebinfo.log` |
| 050-M0-candidate-empty-configure | 0 | 2026-09-22T07:28:09Z | Configure mutation M0 unmutated candidate, build type 'empty' | `logs/050-M0-candidate-empty-configure.log` |
| 050-M0-candidate-empty-build | 0 | 2026-09-22T07:28:10Z | Build only the probe targets of mutation M0 unmutated candidate, capped at 8 jobs | `logs/050-M0-candidate-empty-build.log` |
| 050-M0-candidate-empty-ctest | 0 | 2026-09-22T07:28:11Z | Run the probe CTest cases of mutation M0 unmutated candidate | `logs/050-M0-candidate-empty-ctest.log` |
| 051-M1-base-helper-empty-apply | 0 | 2026-09-22T07:28:11Z | Apply mutation M1 base helper to a copy of the candidate | `logs/051-M1-base-helper-empty-apply.log` |
| 051-M1-base-helper-empty-configure | 0 | 2026-09-22T07:28:12Z | Configure mutation M1 base helper, build type 'empty' | `logs/051-M1-base-helper-empty-configure.log` |
| 051-M1-base-helper-empty-build | 0 | 2026-09-22T07:28:12Z | Build only the probe targets of mutation M1 base helper, capped at 8 jobs | `logs/051-M1-base-helper-empty-build.log` |
| 051-M1-base-helper-empty-ctest | 8 | 2026-09-22T07:28:14Z | Run the probe CTest cases of mutation M1 base helper | `logs/051-M1-base-helper-empty-ctest.log` |
| 052-M1-base-helper-debug-apply | 0 | 2026-09-22T07:28:14Z | Apply mutation M1 base helper to a copy of the candidate | `logs/052-M1-base-helper-debug-apply.log` |
| 052-M1-base-helper-debug-configure | 0 | 2026-09-22T07:28:14Z | Configure mutation M1 base helper, build type 'Debug' | `logs/052-M1-base-helper-debug-configure.log` |
| 052-M1-base-helper-debug-build | 0 | 2026-09-22T07:28:15Z | Build only the probe targets of mutation M1 base helper, capped at 8 jobs | `logs/052-M1-base-helper-debug-build.log` |
| 052-M1-base-helper-debug-ctest | 0 | 2026-09-22T07:28:16Z | Run the probe CTest cases of mutation M1 base helper | `logs/052-M1-base-helper-debug-ctest.log` |
| 053-M2-library-only-empty-apply | 0 | 2026-09-22T07:28:17Z | Apply mutation M2 library-only instrumentation to a copy of the candidate | `logs/053-M2-library-only-empty-apply.log` |
| 053-M2-library-only-empty-configure | 0 | 2026-09-22T07:28:17Z | Configure mutation M2 library-only instrumentation, build type 'empty' | `logs/053-M2-library-only-empty-configure.log` |
| 053-M2-library-only-empty-build | 0 | 2026-09-22T07:28:17Z | Build only the probe targets of mutation M2 library-only instrumentation, capped at 8 jobs | `logs/053-M2-library-only-empty-build.log` |
| 053-M2-library-only-empty-ctest | 8 | 2026-09-22T07:28:19Z | Run the probe CTest cases of mutation M2 library-only instrumentation | `logs/053-M2-library-only-empty-ctest.log` |
| 054-M3-consumer-only-empty-apply | 0 | 2026-09-22T07:28:19Z | Apply mutation M3 consumer-only instrumentation to a copy of the candidate | `logs/054-M3-consumer-only-empty-apply.log` |
| 054-M3-consumer-only-empty-configure | 0 | 2026-09-22T07:28:19Z | Configure mutation M3 consumer-only instrumentation, build type 'empty' | `logs/054-M3-consumer-only-empty-configure.log` |
| 054-M3-consumer-only-empty-build | 0 | 2026-09-22T07:28:20Z | Build only the probe targets of mutation M3 consumer-only instrumentation, capped at 8 jobs | `logs/054-M3-consumer-only-empty-build.log` |
| 054-M3-consumer-only-empty-ctest | 8 | 2026-09-22T07:28:21Z | Run the probe CTest cases of mutation M3 consumer-only instrumentation | `logs/054-M3-consumer-only-empty-ctest.log` |
| 055-M4-no-ubsan-fatal-setting-empty-apply | 0 | 2026-09-22T07:28:22Z | Apply mutation M4 UBSan controls without the fatal setting to a copy of the candidate | `logs/055-M4-no-ubsan-fatal-setting-empty-apply.log` |
| 055-M4-no-ubsan-fatal-setting-empty-configure | 0 | 2026-09-22T07:28:22Z | Configure mutation M4 UBSan controls without the fatal setting, build type 'empty' | `logs/055-M4-no-ubsan-fatal-setting-empty-configure.log` |
| 055-M4-no-ubsan-fatal-setting-empty-build | 0 | 2026-09-22T07:28:22Z | Build only the probe targets of mutation M4 UBSan controls without the fatal setting, capped at 8 jobs | `logs/055-M4-no-ubsan-fatal-setting-empty-build.log` |
| 055-M4-no-ubsan-fatal-setting-empty-ctest | 8 | 2026-09-22T07:28:24Z | Run the probe CTest cases of mutation M4 UBSan controls without the fatal setting | `logs/055-M4-no-ubsan-fatal-setting-empty-ctest.log` |
| 056-M5-no-runtime-at-link-empty-apply | 0 | 2026-09-22T07:28:24Z | Apply mutation M5 no sanitizer runtime at link to a copy of the candidate | `logs/056-M5-no-runtime-at-link-empty-apply.log` |
| 056-M5-no-runtime-at-link-empty-configure | 0 | 2026-09-22T07:28:24Z | Configure mutation M5 no sanitizer runtime at link, build type 'empty' | `logs/056-M5-no-runtime-at-link-empty-configure.log` |
| 056-M5-no-runtime-at-link-empty-build | 2 | 2026-09-22T07:28:25Z | Build only the probe targets of mutation M5 no sanitizer runtime at link, capped at 8 jobs | `logs/056-M5-no-runtime-at-link-empty-build.log` |
| 057-M5-no-runtime-at-link-ubsan-build | 2 | 2026-09-22T07:28:51Z | M5: build the UBSan probe executable alone without its runtime at link | `logs/057-M5-no-runtime-at-link-ubsan-build.log` |
| 060-venv-create | 0 | 2026-09-22T07:29:05Z | Create disposable venv for the documented BDD tool (outside the checkout) | `logs/060-venv-create.log` |
| 061-venv-behave | 0 | 2026-09-22T07:29:07Z | pip install behave into the disposable venv (documented BDD tool) | `logs/061-venv-behave.log` |
| 062-bdd-cand-default-aecp_behave | 0 | 2026-09-22T07:29:23Z | behave --tags=-hardware --tags=-verilator traffic-gen/tests/aecp_behave/features/ with PACKET_GEN_BIN=$VALIDATION_STORAGE/tmp/a162-tsngen15/build/cand-default/traffic-gen/packet_gen | `logs/062-bdd-cand-default-aecp_behave.log` |
| 062-bdd-cand-default-stack_behave | 0 | 2026-09-22T07:29:24Z | behave --tags=-hardware --tags=-verilator traffic-gen/tests/stack_behave/features/ with PACKET_GEN_BIN=$VALIDATION_STORAGE/tmp/a162-tsngen15/build/cand-default/traffic-gen/packet_gen | `logs/062-bdd-cand-default-stack_behave.log` |
| 062-bdd-base-default-aecp_behave | 0 | 2026-09-22T07:29:26Z | behave --tags=-hardware --tags=-verilator traffic-gen/tests/aecp_behave/features/ with PACKET_GEN_BIN=$VALIDATION_STORAGE/tmp/a162-tsngen15/build/base-default/traffic-gen/packet_gen | `logs/062-bdd-base-default-aecp_behave.log` |
| 062-bdd-base-default-stack_behave | 0 | 2026-09-22T07:29:28Z | behave --tags=-hardware --tags=-verilator traffic-gen/tests/stack_behave/features/ with PACKET_GEN_BIN=$VALIDATION_STORAGE/tmp/a162-tsngen15/build/base-default/traffic-gen/packet_gen | `logs/062-bdd-base-default-stack_behave.log` |
| 070-export-replay | 0 | 2026-09-22T07:29:43Z | Fresh export of candidate 76906b8 for the literal workflow replay | `logs/070-export-replay.log` |
| 072-replay-setup | 0 | 2026-09-22T07:29:43Z | Workflow step: ./setup.sh d (make -j$(nproc) = -j8 under this affinity) | `logs/072-replay-setup.log` |
| 073-replay-unit | 0 | 2026-09-22T07:30:04Z | Unit workflow step: ./tests/run-tests.sh | `logs/073-replay-unit.log` |
| 074-replay-behave | 0 | 2026-09-22T07:30:08Z | Behave workflow step: ./tests/run-tests-behave.sh | `logs/074-replay-behave.log` |
| 080-base-tests-off-configure | 0 | 2026-09-22T07:31:00Z | Top-level tsn-gen with ENABLE_PARSER_TESTS=OFF, empty build type | `logs/080-base-tests-off-configure.log` |
| 080-base-tests-off-build | 0 | 2026-09-22T07:31:00Z | Build tests-off tree, capped at 8 jobs | `logs/080-base-tests-off-build.log` |
| 080-base-tests-off-run | 0 | 2026-09-22T07:31:07Z | packet_gen builds one logic-driven AECP ACQUIRE_ENTITY frame | `logs/080-base-tests-off-run.log` |
| 081-base-embedded-empty-configure | 0 | 2026-09-22T07:31:07Z | Embedded add_subdirectory consumer, build type 'empty' | `logs/081-base-embedded-empty-configure.log` |
| 081-base-embedded-empty-build | 0 | 2026-09-22T07:31:08Z | Build embedded consumer, capped at 8 jobs | `logs/081-base-embedded-empty-build.log` |
| 081-base-embedded-empty-run | 0 | 2026-09-22T07:31:15Z | Run the embedded consumer app | `logs/081-base-embedded-empty-run.log` |
| 081-base-embedded-Debug-configure | 0 | 2026-09-22T07:31:15Z | Embedded add_subdirectory consumer, build type 'Debug' | `logs/081-base-embedded-Debug-configure.log` |
| 081-base-embedded-Debug-build | 0 | 2026-09-22T07:31:15Z | Build embedded consumer, capped at 8 jobs | `logs/081-base-embedded-Debug-build.log` |
| 081-base-embedded-Debug-run | 0 | 2026-09-22T07:31:23Z | Run the embedded consumer app | `logs/081-base-embedded-Debug-run.log` |
| 080-cand-tests-off-configure | 0 | 2026-09-22T07:31:23Z | Top-level tsn-gen with ENABLE_PARSER_TESTS=OFF, empty build type | `logs/080-cand-tests-off-configure.log` |
| 080-cand-tests-off-build | 0 | 2026-09-22T07:31:24Z | Build tests-off tree, capped at 8 jobs | `logs/080-cand-tests-off-build.log` |
| 080-cand-tests-off-run | 0 | 2026-09-22T07:31:31Z | packet_gen builds one logic-driven AECP ACQUIRE_ENTITY frame | `logs/080-cand-tests-off-run.log` |
| 081-cand-embedded-empty-configure | 0 | 2026-09-22T07:31:31Z | Embedded add_subdirectory consumer, build type 'empty' | `logs/081-cand-embedded-empty-configure.log` |
| 081-cand-embedded-empty-build | 0 | 2026-09-22T07:31:31Z | Build embedded consumer, capped at 8 jobs | `logs/081-cand-embedded-empty-build.log` |
| 081-cand-embedded-empty-run | 0 | 2026-09-22T07:31:38Z | Run the embedded consumer app | `logs/081-cand-embedded-empty-run.log` |
| 081-cand-embedded-Debug-configure | 0 | 2026-09-22T07:31:38Z | Embedded add_subdirectory consumer, build type 'Debug' | `logs/081-cand-embedded-Debug-configure.log` |
| 081-cand-embedded-Debug-build | 0 | 2026-09-22T07:31:38Z | Build embedded consumer, capped at 8 jobs | `logs/081-cand-embedded-Debug-build.log` |
| 081-cand-embedded-Debug-run | 0 | 2026-09-22T07:31:46Z | Run the embedded consumer app | `logs/081-cand-embedded-Debug-run.log` |
| 082-compare-consumers | 0 | 2026-09-22T07:31:53Z | Compare base and candidate generated files for tests-off and embedded consumers | `logs/082-compare-consumers.log` |
| 083-compare-consumers-neutral | 0 | 2026-09-22T07:32:06Z | Compare base and candidate consumer generated files with the tsn-gen root neutralised | `logs/083-compare-consumers-neutral.log` |
| 084-recompare-base-candidate | 0 | 2026-09-22T07:32:06Z | Re-run the matrix comparison with the updated script (same inputs) | `logs/084-recompare-base-candidate.log` |
| 090-cross-candidate-configure | 0 | 2026-09-22T07:32:52Z | Configure with aarch64-linux-gnu-g++ (cross), empty build type | `logs/090-cross-candidate-configure.log` |
| 090-cross-candidate-libs | 0 | 2026-09-22T07:32:52Z | Build sanitizer variant shared libraries with the cross compiler, 8 jobs | `logs/090-cross-candidate-libs.log` |
| 090-cross-candidate-obj-sanitizer_probe-test_ASAN | 0 | 2026-09-22T07:33:00Z | Compile tests/sanitizer/CMakeFiles/sanitizer_probe-test_ASAN.dir/sanitizer_probe_test.cpp.o only, with the cross compiler | `logs/090-cross-candidate-obj-sanitizer_probe-test_ASAN.log` |
| 090-cross-candidate-obj-sanitizer_probe-test_UBSAN | 0 | 2026-09-22T07:33:01Z | Compile tests/sanitizer/CMakeFiles/sanitizer_probe-test_UBSAN.dir/sanitizer_probe_test.cpp.o only, with the cross compiler | `logs/090-cross-candidate-obj-sanitizer_probe-test_UBSAN.log` |
| 090-cross-candidate-obj-ptp_flags-test_ASAN | 0 | 2026-09-22T07:33:01Z | Compile traffic-gen/tests/CMakeFiles/ptp_flags-test_ASAN.dir/ptp_flags_test.cpp.o only, with the cross compiler | `logs/090-cross-candidate-obj-ptp_flags-test_ASAN.log` |
| 090-cross-candidate-obj-ptp_flags-test_UBSAN | 0 | 2026-09-22T07:33:03Z | Compile traffic-gen/tests/CMakeFiles/ptp_flags-test_UBSAN.dir/ptp_flags_test.cpp.o only, with the cross compiler | `logs/090-cross-candidate-obj-ptp_flags-test_UBSAN.log` |
| 090-cross-candidate-exe-link | 2 | 2026-09-22T07:33:05Z | Attempt the sanitizer_probe-test_ASAN link with the cross compiler | `logs/090-cross-candidate-exe-link.log` |
| 091-cross-M1-base-helper-configure | 0 | 2026-09-22T07:33:06Z | Configure with aarch64-linux-gnu-g++ (cross), empty build type | `logs/091-cross-M1-base-helper-configure.log` |
| 091-cross-M1-base-helper-libs | 0 | 2026-09-22T07:33:07Z | Build sanitizer variant shared libraries with the cross compiler, 8 jobs | `logs/091-cross-M1-base-helper-libs.log` |
| 091-cross-M1-base-helper-obj-sanitizer_probe-test_ASAN | 0 | 2026-09-22T07:33:14Z | Compile tests/sanitizer/CMakeFiles/sanitizer_probe-test_ASAN.dir/sanitizer_probe_test.cpp.o only, with the cross compiler | `logs/091-cross-M1-base-helper-obj-sanitizer_probe-test_ASAN.log` |
| 091-cross-M1-base-helper-obj-sanitizer_probe-test_UBSAN | 0 | 2026-09-22T07:33:14Z | Compile tests/sanitizer/CMakeFiles/sanitizer_probe-test_UBSAN.dir/sanitizer_probe_test.cpp.o only, with the cross compiler | `logs/091-cross-M1-base-helper-obj-sanitizer_probe-test_UBSAN.log` |
| 091-cross-M1-base-helper-obj-ptp_flags-test_ASAN | 0 | 2026-09-22T07:33:14Z | Compile traffic-gen/tests/CMakeFiles/ptp_flags-test_ASAN.dir/ptp_flags_test.cpp.o only, with the cross compiler | `logs/091-cross-M1-base-helper-obj-ptp_flags-test_ASAN.log` |
| 091-cross-M1-base-helper-obj-ptp_flags-test_UBSAN | 0 | 2026-09-22T07:33:16Z | Compile traffic-gen/tests/CMakeFiles/ptp_flags-test_UBSAN.dir/ptp_flags_test.cpp.o only, with the cross compiler | `logs/091-cross-M1-base-helper-obj-ptp_flags-test_UBSAN.log` |
| 091-cross-M1-base-helper-exe-link | 2 | 2026-09-22T07:33:18Z | Attempt the sanitizer_probe-test_ASAN link with the cross compiler | `logs/091-cross-M1-base-helper-exe-link.log` |
| 085-custom-type-base | 0 | 2026-09-22T07:34:49Z | Configure-only: custom CMAKE_BUILD_TYPE=Profile, base | `logs/085-custom-type-base.log` |
| 085-custom-type-cand | 0 | 2026-09-22T07:34:50Z | Configure-only: custom CMAKE_BUILD_TYPE=Profile, cand | `logs/085-custom-type-cand.log` |

## Full argv

- `001-submodule-googletest` (cwd `$VALIDATION_STORAGE/lanes/tsngen15-sanitizer-targets`): `git submodule update --init external/googletest`
- `002-submodule-rapidyaml` (cwd `$VALIDATION_STORAGE/lanes/tsngen15-sanitizer-targets`): `git submodule update --init --recursive external/rapidyaml`
- `003-configure-matrix-base` (cwd `$VALIDATION_STORAGE/lanes/tsngen15-sanitizer-targets`): `$WORKSPACE_HOME/milan-fpga-management/2026-09-22/tsngen15-author/scripts/configure-matrix.sh base $VALIDATION_STORAGE/lanes/tsngen15-sanitizer-targets $VALIDATION_STORAGE/tmp/a162-tsngen15/cfg $WORKSPACE_HOME/milan-fpga-management/2026-09-22/tsngen15-author/receipts/configure`
- `004-export-base` (cwd `$VALIDATION_STORAGE/lanes/tsngen15-sanitizer-targets`): `$WORKSPACE_HOME/milan-fpga-management/2026-09-22/tsngen15-author/scripts/export-tree.sh $VALIDATION_STORAGE/lanes/tsngen15-sanitizer-targets deca300c9eb4863fa13383b45ba6cb6fd0828671 $VALIDATION_STORAGE/tmp/a162-tsngen15/src/base`
- `010-base-default-configure` (cwd `$VALIDATION_STORAGE/tmp/a162-tsngen15/build/base-default`): `env -u CMAKE_BUILD_TYPE cmake $VALIDATION_STORAGE/tmp/a162-tsngen15/src/base -DBUILD_SHARED_LIBS=ON`
- `010-base-default-build` (cwd `$VALIDATION_STORAGE/tmp/a162-tsngen15/build/base-default`): `make -j8`
- `010-base-default-prep` (cwd `$VALIDATION_STORAGE/tmp/a162-tsngen15/build/base-default`): `$VALIDATION_STORAGE/tmp/a162-tsngen15/src/base/parser/tests/prep-tests.sh`
- `010-base-default-unit` (cwd `$VALIDATION_STORAGE/tmp/a162-tsngen15/build/base-default`): `$VALIDATION_STORAGE/tmp/a162-tsngen15/src/base/tests/run-tests.sh $VALIDATION_STORAGE/tmp/a162-tsngen15/build/base-default`
- `011-configure-matrix-helper-only` (cwd `$VALIDATION_STORAGE/lanes/tsngen15-sanitizer-targets`): `$WORKSPACE_HOME/milan-fpga-management/2026-09-22/tsngen15-author/scripts/configure-matrix.sh helper-only $VALIDATION_STORAGE/lanes/tsngen15-sanitizer-targets $VALIDATION_STORAGE/tmp/a162-tsngen15/cfg $WORKSPACE_HOME/milan-fpga-management/2026-09-22/tsngen15-author/receipts/configure`
- `020-configure-matrix-candidate` (cwd `$VALIDATION_STORAGE/lanes/tsngen15-sanitizer-targets`): `$WORKSPACE_HOME/milan-fpga-management/2026-09-22/tsngen15-author/scripts/configure-matrix.sh cand $VALIDATION_STORAGE/lanes/tsngen15-sanitizer-targets $VALIDATION_STORAGE/tmp/a162-tsngen15/cfg $WORKSPACE_HOME/milan-fpga-management/2026-09-22/tsngen15-author/receipts/configure`
- `021-compare-base-candidate` (cwd `$WORKSPACE_HOME/milan-fpga-management/2026-09-22/tsngen15-author`): `python3 scripts/compare-configure.py receipts/configure base cand receipts/compare-base-vs-candidate.json`
- `030-export-candidate` (cwd `$VALIDATION_STORAGE/lanes/tsngen15-sanitizer-targets`): `$WORKSPACE_HOME/milan-fpga-management/2026-09-22/tsngen15-author/scripts/export-tree.sh $VALIDATION_STORAGE/lanes/tsngen15-sanitizer-targets 76906b83eb54ac29040fe72fd910d34727188694 $VALIDATION_STORAGE/tmp/a162-tsngen15/src/cand`
- `031-cand-default-configure` (cwd `$VALIDATION_STORAGE/tmp/a162-tsngen15/build/cand-default`): `env -u CMAKE_BUILD_TYPE cmake $VALIDATION_STORAGE/tmp/a162-tsngen15/src/cand -DBUILD_SHARED_LIBS=ON`
- `031-cand-default-build` (cwd `$VALIDATION_STORAGE/tmp/a162-tsngen15/build/cand-default`): `make -j8`
- `031-cand-default-prep` (cwd `$VALIDATION_STORAGE/tmp/a162-tsngen15/build/cand-default`): `$VALIDATION_STORAGE/tmp/a162-tsngen15/src/cand/parser/tests/prep-tests.sh`
- `031-cand-default-unit` (cwd `$VALIDATION_STORAGE/tmp/a162-tsngen15/build/cand-default`): `$VALIDATION_STORAGE/tmp/a162-tsngen15/src/cand/tests/run-tests.sh $VALIDATION_STORAGE/tmp/a162-tsngen15/build/cand-default`
- `032-scan-base-default` (cwd `$WORKSPACE_HOME/milan-fpga-management/2026-09-22/tsngen15-author`): `python3 scripts/scan-test-log.py $VALIDATION_STORAGE/tmp/a162-tsngen15/build/base-default receipts/scan-base-default.json`
- `032-scan-cand-default` (cwd `$WORKSPACE_HOME/milan-fpga-management/2026-09-22/tsngen15-author`): `python3 scripts/scan-test-log.py $VALIDATION_STORAGE/tmp/a162-tsngen15/build/cand-default receipts/scan-cand-default.json`
- `033-instrumentation-base-default` (cwd `$WORKSPACE_HOME/milan-fpga-management/2026-09-22/tsngen15-author`): `python3 scripts/instrumentation.py $VALIDATION_STORAGE/tmp/a162-tsngen15/build/base-default receipts/instrumentation-base-default.json`
- `033-instrumentation-cand-default` (cwd `$WORKSPACE_HOME/milan-fpga-management/2026-09-22/tsngen15-author`): `python3 scripts/instrumentation.py $VALIDATION_STORAGE/tmp/a162-tsngen15/build/cand-default receipts/instrumentation-cand-default.json`
- `034-instrumentation-base-default` (cwd `$WORKSPACE_HOME/milan-fpga-management/2026-09-22/tsngen15-author`): `python3 scripts/instrumentation.py $VALIDATION_STORAGE/tmp/a162-tsngen15/build/base-default receipts/instrumentation-base-default.json`
- `034-instrumentation-cand-default` (cwd `$WORKSPACE_HOME/milan-fpga-management/2026-09-22/tsngen15-author`): `python3 scripts/instrumentation.py $VALIDATION_STORAGE/tmp/a162-tsngen15/build/cand-default receipts/instrumentation-cand-default.json`
- `040-base-debug-configure` (cwd `$VALIDATION_STORAGE/lanes/tsngen15-sanitizer-targets`): `env -u CMAKE_BUILD_TYPE cmake -S $VALIDATION_STORAGE/tmp/a162-tsngen15/src/base -B $VALIDATION_STORAGE/tmp/a162-tsngen15/build/base-debug -DCMAKE_BUILD_TYPE=Debug`
- `040-base-debug-build` (cwd `$VALIDATION_STORAGE/lanes/tsngen15-sanitizer-targets`): `cmake --build $VALIDATION_STORAGE/tmp/a162-tsngen15/build/base-debug -j8`
- `040-base-debug-unit` (cwd `$VALIDATION_STORAGE/lanes/tsngen15-sanitizer-targets`): `$VALIDATION_STORAGE/tmp/a162-tsngen15/src/base/tests/run-tests.sh $VALIDATION_STORAGE/tmp/a162-tsngen15/build/base-debug`
- `040-cand-debug-configure` (cwd `$VALIDATION_STORAGE/lanes/tsngen15-sanitizer-targets`): `env -u CMAKE_BUILD_TYPE cmake -S $VALIDATION_STORAGE/tmp/a162-tsngen15/src/cand -B $VALIDATION_STORAGE/tmp/a162-tsngen15/build/cand-debug -DCMAKE_BUILD_TYPE=Debug`
- `040-cand-debug-build` (cwd `$VALIDATION_STORAGE/lanes/tsngen15-sanitizer-targets`): `cmake --build $VALIDATION_STORAGE/tmp/a162-tsngen15/build/cand-debug -j8`
- `040-cand-debug-unit` (cwd `$VALIDATION_STORAGE/lanes/tsngen15-sanitizer-targets`): `$VALIDATION_STORAGE/tmp/a162-tsngen15/src/cand/tests/run-tests.sh $VALIDATION_STORAGE/tmp/a162-tsngen15/build/cand-debug`
- `041-scan-base-debug` (cwd `$WORKSPACE_HOME/milan-fpga-management/2026-09-22/tsngen15-author`): `python3 scripts/scan-test-log.py $VALIDATION_STORAGE/tmp/a162-tsngen15/build/base-debug receipts/scan-base-debug.json`
- `042-instrumentation-base-debug` (cwd `$WORKSPACE_HOME/milan-fpga-management/2026-09-22/tsngen15-author`): `python3 scripts/instrumentation.py $VALIDATION_STORAGE/tmp/a162-tsngen15/build/base-debug receipts/instrumentation-base-debug.json`
- `041-scan-cand-debug` (cwd `$WORKSPACE_HOME/milan-fpga-management/2026-09-22/tsngen15-author`): `python3 scripts/scan-test-log.py $VALIDATION_STORAGE/tmp/a162-tsngen15/build/cand-debug receipts/scan-cand-debug.json`
- `042-instrumentation-cand-debug` (cwd `$WORKSPACE_HOME/milan-fpga-management/2026-09-22/tsngen15-author`): `python3 scripts/instrumentation.py $VALIDATION_STORAGE/tmp/a162-tsngen15/build/cand-debug receipts/instrumentation-cand-debug.json`
- `043-cand-release-configure` (cwd `$VALIDATION_STORAGE/lanes/tsngen15-sanitizer-targets`): `env -u CMAKE_BUILD_TYPE cmake -S $VALIDATION_STORAGE/tmp/a162-tsngen15/src/cand -B $VALIDATION_STORAGE/tmp/a162-tsngen15/build/cand-release -DCMAKE_BUILD_TYPE=Release`
- `043-cand-release-build` (cwd `$VALIDATION_STORAGE/lanes/tsngen15-sanitizer-targets`): `cmake --build $VALIDATION_STORAGE/tmp/a162-tsngen15/build/cand-release -j8`
- `043-cand-release-unit` (cwd `$VALIDATION_STORAGE/lanes/tsngen15-sanitizer-targets`): `$VALIDATION_STORAGE/tmp/a162-tsngen15/src/cand/tests/run-tests.sh $VALIDATION_STORAGE/tmp/a162-tsngen15/build/cand-release`
- `043-cand-minsizerel-configure` (cwd `$VALIDATION_STORAGE/lanes/tsngen15-sanitizer-targets`): `env -u CMAKE_BUILD_TYPE cmake -S $VALIDATION_STORAGE/tmp/a162-tsngen15/src/cand -B $VALIDATION_STORAGE/tmp/a162-tsngen15/build/cand-minsizerel -DCMAKE_BUILD_TYPE=MinSizeRel`
- `043-cand-minsizerel-build` (cwd `$VALIDATION_STORAGE/lanes/tsngen15-sanitizer-targets`): `cmake --build $VALIDATION_STORAGE/tmp/a162-tsngen15/build/cand-minsizerel -j8`
- `043-cand-minsizerel-unit` (cwd `$VALIDATION_STORAGE/lanes/tsngen15-sanitizer-targets`): `$VALIDATION_STORAGE/tmp/a162-tsngen15/src/cand/tests/run-tests.sh $VALIDATION_STORAGE/tmp/a162-tsngen15/build/cand-minsizerel`
- `043-cand-relwithdebinfo-configure` (cwd `$VALIDATION_STORAGE/lanes/tsngen15-sanitizer-targets`): `env -u CMAKE_BUILD_TYPE cmake -S $VALIDATION_STORAGE/tmp/a162-tsngen15/src/cand -B $VALIDATION_STORAGE/tmp/a162-tsngen15/build/cand-relwithdebinfo -DCMAKE_BUILD_TYPE=RelWithDebInfo`
- `043-cand-relwithdebinfo-build` (cwd `$VALIDATION_STORAGE/lanes/tsngen15-sanitizer-targets`): `cmake --build $VALIDATION_STORAGE/tmp/a162-tsngen15/build/cand-relwithdebinfo -j8`
- `043-cand-relwithdebinfo-unit` (cwd `$VALIDATION_STORAGE/lanes/tsngen15-sanitizer-targets`): `$VALIDATION_STORAGE/tmp/a162-tsngen15/src/cand/tests/run-tests.sh $VALIDATION_STORAGE/tmp/a162-tsngen15/build/cand-relwithdebinfo`
- `044-scan-cand-release` (cwd `$WORKSPACE_HOME/milan-fpga-management/2026-09-22/tsngen15-author`): `python3 scripts/scan-test-log.py $VALIDATION_STORAGE/tmp/a162-tsngen15/build/cand-release receipts/scan-cand-release.json`
- `045-instrumentation-cand-release` (cwd `$WORKSPACE_HOME/milan-fpga-management/2026-09-22/tsngen15-author`): `python3 scripts/instrumentation.py $VALIDATION_STORAGE/tmp/a162-tsngen15/build/cand-release receipts/instrumentation-cand-release.json`
- `044-scan-cand-minsizerel` (cwd `$WORKSPACE_HOME/milan-fpga-management/2026-09-22/tsngen15-author`): `python3 scripts/scan-test-log.py $VALIDATION_STORAGE/tmp/a162-tsngen15/build/cand-minsizerel receipts/scan-cand-minsizerel.json`
- `045-instrumentation-cand-minsizerel` (cwd `$WORKSPACE_HOME/milan-fpga-management/2026-09-22/tsngen15-author`): `python3 scripts/instrumentation.py $VALIDATION_STORAGE/tmp/a162-tsngen15/build/cand-minsizerel receipts/instrumentation-cand-minsizerel.json`
- `044-scan-cand-relwithdebinfo` (cwd `$WORKSPACE_HOME/milan-fpga-management/2026-09-22/tsngen15-author`): `python3 scripts/scan-test-log.py $VALIDATION_STORAGE/tmp/a162-tsngen15/build/cand-relwithdebinfo receipts/scan-cand-relwithdebinfo.json`
- `045-instrumentation-cand-relwithdebinfo` (cwd `$WORKSPACE_HOME/milan-fpga-management/2026-09-22/tsngen15-author`): `python3 scripts/instrumentation.py $VALIDATION_STORAGE/tmp/a162-tsngen15/build/cand-relwithdebinfo receipts/instrumentation-cand-relwithdebinfo.json`
- `050-M0-candidate-empty-configure` (cwd `$WORKSPACE_HOME/milan-fpga-management/2026-09-22/tsngen15-author`): `env -u CMAKE_BUILD_TYPE cmake -S $VALIDATION_STORAGE/tmp/a162-tsngen15/src/mut-050-M0-candidate-empty -B $VALIDATION_STORAGE/tmp/a162-tsngen15/build/mut-050-M0-candidate-empty -G 'Unix Makefiles' -DBUILD_SHARED_LIBS=ON -DCMAKE_EXPORT_COMPILE_COMMANDS=ON`
- `050-M0-candidate-empty-build` (cwd `$WORKSPACE_HOME/milan-fpga-management/2026-09-22/tsngen15-author`): `cmake --build $VALIDATION_STORAGE/tmp/a162-tsngen15/build/mut-050-M0-candidate-empty -j8 --target sanitizer_probe-test sanitizer_probe-test_ASAN sanitizer_probe-test_UBSAN`
- `050-M0-candidate-empty-ctest` (cwd `$WORKSPACE_HOME/milan-fpga-management/2026-09-22/tsngen15-author`): `ctest --test-dir $VALIDATION_STORAGE/tmp/a162-tsngen15/build/mut-050-M0-candidate-empty -R '^sanitizer_probe' --output-on-failure`
- `051-M1-base-helper-empty-apply` (cwd `$WORKSPACE_HOME/milan-fpga-management/2026-09-22/tsngen15-author`): `patch -p1 -d $VALIDATION_STORAGE/tmp/a162-tsngen15/src/mut-051-M1-base-helper-empty -i $WORKSPACE_HOME/milan-fpga-management/2026-09-22/tsngen15-author/receipts/mutations/M1-base-helper.patch`
- `051-M1-base-helper-empty-configure` (cwd `$WORKSPACE_HOME/milan-fpga-management/2026-09-22/tsngen15-author`): `env -u CMAKE_BUILD_TYPE cmake -S $VALIDATION_STORAGE/tmp/a162-tsngen15/src/mut-051-M1-base-helper-empty -B $VALIDATION_STORAGE/tmp/a162-tsngen15/build/mut-051-M1-base-helper-empty -G 'Unix Makefiles' -DBUILD_SHARED_LIBS=ON -DCMAKE_EXPORT_COMPILE_COMMANDS=ON`
- `051-M1-base-helper-empty-build` (cwd `$WORKSPACE_HOME/milan-fpga-management/2026-09-22/tsngen15-author`): `cmake --build $VALIDATION_STORAGE/tmp/a162-tsngen15/build/mut-051-M1-base-helper-empty -j8 --target sanitizer_probe-test sanitizer_probe-test_ASAN sanitizer_probe-test_UBSAN`
- `051-M1-base-helper-empty-ctest` (cwd `$WORKSPACE_HOME/milan-fpga-management/2026-09-22/tsngen15-author`): `ctest --test-dir $VALIDATION_STORAGE/tmp/a162-tsngen15/build/mut-051-M1-base-helper-empty -R '^sanitizer_probe' --output-on-failure`
- `052-M1-base-helper-debug-apply` (cwd `$WORKSPACE_HOME/milan-fpga-management/2026-09-22/tsngen15-author`): `patch -p1 -d $VALIDATION_STORAGE/tmp/a162-tsngen15/src/mut-052-M1-base-helper-debug -i $WORKSPACE_HOME/milan-fpga-management/2026-09-22/tsngen15-author/receipts/mutations/M1-base-helper.patch`
- `052-M1-base-helper-debug-configure` (cwd `$WORKSPACE_HOME/milan-fpga-management/2026-09-22/tsngen15-author`): `env -u CMAKE_BUILD_TYPE cmake -S $VALIDATION_STORAGE/tmp/a162-tsngen15/src/mut-052-M1-base-helper-debug -B $VALIDATION_STORAGE/tmp/a162-tsngen15/build/mut-052-M1-base-helper-debug -G 'Unix Makefiles' -DBUILD_SHARED_LIBS=ON -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_BUILD_TYPE=Debug`
- `052-M1-base-helper-debug-build` (cwd `$WORKSPACE_HOME/milan-fpga-management/2026-09-22/tsngen15-author`): `cmake --build $VALIDATION_STORAGE/tmp/a162-tsngen15/build/mut-052-M1-base-helper-debug -j8 --target sanitizer_probe-test sanitizer_probe-test_ASAN sanitizer_probe-test_UBSAN`
- `052-M1-base-helper-debug-ctest` (cwd `$WORKSPACE_HOME/milan-fpga-management/2026-09-22/tsngen15-author`): `ctest --test-dir $VALIDATION_STORAGE/tmp/a162-tsngen15/build/mut-052-M1-base-helper-debug -R '^sanitizer_probe' --output-on-failure`
- `053-M2-library-only-empty-apply` (cwd `$WORKSPACE_HOME/milan-fpga-management/2026-09-22/tsngen15-author`): `patch -p1 -d $VALIDATION_STORAGE/tmp/a162-tsngen15/src/mut-053-M2-library-only-empty -i $WORKSPACE_HOME/milan-fpga-management/2026-09-22/tsngen15-author/receipts/mutations/M2-library-only.patch`
- `053-M2-library-only-empty-configure` (cwd `$WORKSPACE_HOME/milan-fpga-management/2026-09-22/tsngen15-author`): `env -u CMAKE_BUILD_TYPE cmake -S $VALIDATION_STORAGE/tmp/a162-tsngen15/src/mut-053-M2-library-only-empty -B $VALIDATION_STORAGE/tmp/a162-tsngen15/build/mut-053-M2-library-only-empty -G 'Unix Makefiles' -DBUILD_SHARED_LIBS=ON -DCMAKE_EXPORT_COMPILE_COMMANDS=ON`
- `053-M2-library-only-empty-build` (cwd `$WORKSPACE_HOME/milan-fpga-management/2026-09-22/tsngen15-author`): `cmake --build $VALIDATION_STORAGE/tmp/a162-tsngen15/build/mut-053-M2-library-only-empty -j8 --target sanitizer_probe-test sanitizer_probe-test_ASAN sanitizer_probe-test_UBSAN`
- `053-M2-library-only-empty-ctest` (cwd `$WORKSPACE_HOME/milan-fpga-management/2026-09-22/tsngen15-author`): `ctest --test-dir $VALIDATION_STORAGE/tmp/a162-tsngen15/build/mut-053-M2-library-only-empty -R '^sanitizer_probe' --output-on-failure`
- `054-M3-consumer-only-empty-apply` (cwd `$WORKSPACE_HOME/milan-fpga-management/2026-09-22/tsngen15-author`): `patch -p1 -d $VALIDATION_STORAGE/tmp/a162-tsngen15/src/mut-054-M3-consumer-only-empty -i $WORKSPACE_HOME/milan-fpga-management/2026-09-22/tsngen15-author/receipts/mutations/M3-consumer-only.patch`
- `054-M3-consumer-only-empty-configure` (cwd `$WORKSPACE_HOME/milan-fpga-management/2026-09-22/tsngen15-author`): `env -u CMAKE_BUILD_TYPE cmake -S $VALIDATION_STORAGE/tmp/a162-tsngen15/src/mut-054-M3-consumer-only-empty -B $VALIDATION_STORAGE/tmp/a162-tsngen15/build/mut-054-M3-consumer-only-empty -G 'Unix Makefiles' -DBUILD_SHARED_LIBS=ON -DCMAKE_EXPORT_COMPILE_COMMANDS=ON`
- `054-M3-consumer-only-empty-build` (cwd `$WORKSPACE_HOME/milan-fpga-management/2026-09-22/tsngen15-author`): `cmake --build $VALIDATION_STORAGE/tmp/a162-tsngen15/build/mut-054-M3-consumer-only-empty -j8 --target sanitizer_probe-test sanitizer_probe-test_ASAN sanitizer_probe-test_UBSAN`
- `054-M3-consumer-only-empty-ctest` (cwd `$WORKSPACE_HOME/milan-fpga-management/2026-09-22/tsngen15-author`): `ctest --test-dir $VALIDATION_STORAGE/tmp/a162-tsngen15/build/mut-054-M3-consumer-only-empty -R '^sanitizer_probe' --output-on-failure`
- `055-M4-no-ubsan-fatal-setting-empty-apply` (cwd `$WORKSPACE_HOME/milan-fpga-management/2026-09-22/tsngen15-author`): `patch -p1 -d $VALIDATION_STORAGE/tmp/a162-tsngen15/src/mut-055-M4-no-ubsan-fatal-setting-empty -i $WORKSPACE_HOME/milan-fpga-management/2026-09-22/tsngen15-author/receipts/mutations/M4-no-ubsan-fatal-setting.patch`
- `055-M4-no-ubsan-fatal-setting-empty-configure` (cwd `$WORKSPACE_HOME/milan-fpga-management/2026-09-22/tsngen15-author`): `env -u CMAKE_BUILD_TYPE cmake -S $VALIDATION_STORAGE/tmp/a162-tsngen15/src/mut-055-M4-no-ubsan-fatal-setting-empty -B $VALIDATION_STORAGE/tmp/a162-tsngen15/build/mut-055-M4-no-ubsan-fatal-setting-empty -G 'Unix Makefiles' -DBUILD_SHARED_LIBS=ON -DCMAKE_EXPORT_COMPILE_COMMANDS=ON`
- `055-M4-no-ubsan-fatal-setting-empty-build` (cwd `$WORKSPACE_HOME/milan-fpga-management/2026-09-22/tsngen15-author`): `cmake --build $VALIDATION_STORAGE/tmp/a162-tsngen15/build/mut-055-M4-no-ubsan-fatal-setting-empty -j8 --target sanitizer_probe-test sanitizer_probe-test_ASAN sanitizer_probe-test_UBSAN`
- `055-M4-no-ubsan-fatal-setting-empty-ctest` (cwd `$WORKSPACE_HOME/milan-fpga-management/2026-09-22/tsngen15-author`): `ctest --test-dir $VALIDATION_STORAGE/tmp/a162-tsngen15/build/mut-055-M4-no-ubsan-fatal-setting-empty -R '^sanitizer_probe' --output-on-failure`
- `056-M5-no-runtime-at-link-empty-apply` (cwd `$WORKSPACE_HOME/milan-fpga-management/2026-09-22/tsngen15-author`): `patch -p1 -d $VALIDATION_STORAGE/tmp/a162-tsngen15/src/mut-056-M5-no-runtime-at-link-empty -i $WORKSPACE_HOME/milan-fpga-management/2026-09-22/tsngen15-author/receipts/mutations/M5-no-runtime-at-link.patch`
- `056-M5-no-runtime-at-link-empty-configure` (cwd `$WORKSPACE_HOME/milan-fpga-management/2026-09-22/tsngen15-author`): `env -u CMAKE_BUILD_TYPE cmake -S $VALIDATION_STORAGE/tmp/a162-tsngen15/src/mut-056-M5-no-runtime-at-link-empty -B $VALIDATION_STORAGE/tmp/a162-tsngen15/build/mut-056-M5-no-runtime-at-link-empty -G 'Unix Makefiles' -DBUILD_SHARED_LIBS=ON -DCMAKE_EXPORT_COMPILE_COMMANDS=ON`
- `056-M5-no-runtime-at-link-empty-build` (cwd `$WORKSPACE_HOME/milan-fpga-management/2026-09-22/tsngen15-author`): `cmake --build $VALIDATION_STORAGE/tmp/a162-tsngen15/build/mut-056-M5-no-runtime-at-link-empty -j8 --target sanitizer_probe-test sanitizer_probe-test_ASAN sanitizer_probe-test_UBSAN`
- `057-M5-no-runtime-at-link-ubsan-build` (cwd `$VALIDATION_STORAGE/lanes/tsngen15-sanitizer-targets`): `cmake --build $VALIDATION_STORAGE/tmp/a162-tsngen15/build/mut-056-M5-no-runtime-at-link-empty -j8 --target sanitizer_probe-test_UBSAN`
- `060-venv-create` (cwd `$VALIDATION_STORAGE/tmp/a162-tsngen15`): `python3 -m venv $VALIDATION_STORAGE/tmp/a162-tsngen15/venv`
- `061-venv-behave` (cwd `$VALIDATION_STORAGE/tmp/a162-tsngen15`): `$VALIDATION_STORAGE/tmp/a162-tsngen15/venv/bin/pip install behave`
- `062-bdd-cand-default-aecp_behave` (cwd `$VALIDATION_STORAGE/tmp/a162-tsngen15/src/cand`): `$VALIDATION_STORAGE/tmp/a162-tsngen15/venv/bin/behave --tags=-hardware --tags=-verilator traffic-gen/tests/aecp_behave/features/`
- `062-bdd-cand-default-stack_behave` (cwd `$VALIDATION_STORAGE/tmp/a162-tsngen15/src/cand`): `$VALIDATION_STORAGE/tmp/a162-tsngen15/venv/bin/behave --tags=-hardware --tags=-verilator traffic-gen/tests/stack_behave/features/`
- `062-bdd-base-default-aecp_behave` (cwd `$VALIDATION_STORAGE/tmp/a162-tsngen15/src/base`): `$VALIDATION_STORAGE/tmp/a162-tsngen15/venv/bin/behave --tags=-hardware --tags=-verilator traffic-gen/tests/aecp_behave/features/`
- `062-bdd-base-default-stack_behave` (cwd `$VALIDATION_STORAGE/tmp/a162-tsngen15/src/base`): `$VALIDATION_STORAGE/tmp/a162-tsngen15/venv/bin/behave --tags=-hardware --tags=-verilator traffic-gen/tests/stack_behave/features/`
- `070-export-replay` (cwd `$VALIDATION_STORAGE/lanes/tsngen15-sanitizer-targets`): `$WORKSPACE_HOME/milan-fpga-management/2026-09-22/tsngen15-author/scripts/export-tree.sh $VALIDATION_STORAGE/lanes/tsngen15-sanitizer-targets 76906b83eb54ac29040fe72fd910d34727188694 $VALIDATION_STORAGE/tmp/a162-tsngen15/src/replay-cand`
- `072-replay-setup` (cwd `$VALIDATION_STORAGE/tmp/a162-tsngen15/src/replay-cand`): `./setup.sh d`
- `073-replay-unit` (cwd `$VALIDATION_STORAGE/tmp/a162-tsngen15/src/replay-cand`): `./tests/run-tests.sh`
- `074-replay-behave` (cwd `$VALIDATION_STORAGE/tmp/a162-tsngen15/src/replay-cand`): `./tests/run-tests-behave.sh`
- `080-base-tests-off-configure` (cwd `$VALIDATION_STORAGE/lanes/tsngen15-sanitizer-targets`): `env -u CMAKE_BUILD_TYPE cmake -S $VALIDATION_STORAGE/tmp/a162-tsngen15/src/base -B $VALIDATION_STORAGE/tmp/a162-tsngen15/build/consumer-base-tests-off -G 'Unix Makefiles' -DENABLE_PARSER_TESTS=OFF -DCMAKE_EXPORT_COMPILE_COMMANDS=ON`
- `080-base-tests-off-build` (cwd `$VALIDATION_STORAGE/lanes/tsngen15-sanitizer-targets`): `cmake --build $VALIDATION_STORAGE/tmp/a162-tsngen15/build/consumer-base-tests-off -j8`
- `080-base-tests-off-run` (cwd `$VALIDATION_STORAGE/lanes/tsngen15-sanitizer-targets`): `$VALIDATION_STORAGE/tmp/a162-tsngen15/build/consumer-base-tests-off/traffic-gen/packet_gen --yaml-dir $VALIDATION_STORAGE/tmp/a162-tsngen15/src/base/protocols --stack-file $VALIDATION_STORAGE/tmp/a162-tsngen15/src/base/stacks/aecp_acquire_entity.yaml --seed 42 --count 1`
- `081-base-embedded-empty-configure` (cwd `$VALIDATION_STORAGE/lanes/tsngen15-sanitizer-targets`): `env -u CMAKE_BUILD_TYPE cmake -S $WORKSPACE_HOME/milan-fpga-management/2026-09-22/tsngen15-author/scripts/embedded-consumer -B $VALIDATION_STORAGE/tmp/a162-tsngen15/build/consumer-base-embedded-empty -G 'Unix Makefiles' -DTSN_GEN_SOURCE_DIR=$VALIDATION_STORAGE/tmp/a162-tsngen15/src/base -DCMAKE_EXPORT_COMPILE_COMMANDS=ON`
- `081-base-embedded-empty-build` (cwd `$VALIDATION_STORAGE/lanes/tsngen15-sanitizer-targets`): `cmake --build $VALIDATION_STORAGE/tmp/a162-tsngen15/build/consumer-base-embedded-empty -j8`
- `081-base-embedded-empty-run` (cwd `$VALIDATION_STORAGE/lanes/tsngen15-sanitizer-targets`): `$VALIDATION_STORAGE/tmp/a162-tsngen15/build/consumer-base-embedded-empty/app`
- `081-base-embedded-Debug-configure` (cwd `$VALIDATION_STORAGE/lanes/tsngen15-sanitizer-targets`): `env -u CMAKE_BUILD_TYPE cmake -S $WORKSPACE_HOME/milan-fpga-management/2026-09-22/tsngen15-author/scripts/embedded-consumer -B $VALIDATION_STORAGE/tmp/a162-tsngen15/build/consumer-base-embedded-Debug -G 'Unix Makefiles' -DTSN_GEN_SOURCE_DIR=$VALIDATION_STORAGE/tmp/a162-tsngen15/src/base -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_BUILD_TYPE=Debug`
- `081-base-embedded-Debug-build` (cwd `$VALIDATION_STORAGE/lanes/tsngen15-sanitizer-targets`): `cmake --build $VALIDATION_STORAGE/tmp/a162-tsngen15/build/consumer-base-embedded-Debug -j8`
- `081-base-embedded-Debug-run` (cwd `$VALIDATION_STORAGE/lanes/tsngen15-sanitizer-targets`): `$VALIDATION_STORAGE/tmp/a162-tsngen15/build/consumer-base-embedded-Debug/app`
- `080-cand-tests-off-configure` (cwd `$VALIDATION_STORAGE/lanes/tsngen15-sanitizer-targets`): `env -u CMAKE_BUILD_TYPE cmake -S $VALIDATION_STORAGE/tmp/a162-tsngen15/src/cand -B $VALIDATION_STORAGE/tmp/a162-tsngen15/build/consumer-cand-tests-off -G 'Unix Makefiles' -DENABLE_PARSER_TESTS=OFF -DCMAKE_EXPORT_COMPILE_COMMANDS=ON`
- `080-cand-tests-off-build` (cwd `$VALIDATION_STORAGE/lanes/tsngen15-sanitizer-targets`): `cmake --build $VALIDATION_STORAGE/tmp/a162-tsngen15/build/consumer-cand-tests-off -j8`
- `080-cand-tests-off-run` (cwd `$VALIDATION_STORAGE/lanes/tsngen15-sanitizer-targets`): `$VALIDATION_STORAGE/tmp/a162-tsngen15/build/consumer-cand-tests-off/traffic-gen/packet_gen --yaml-dir $VALIDATION_STORAGE/tmp/a162-tsngen15/src/cand/protocols --stack-file $VALIDATION_STORAGE/tmp/a162-tsngen15/src/cand/stacks/aecp_acquire_entity.yaml --seed 42 --count 1`
- `081-cand-embedded-empty-configure` (cwd `$VALIDATION_STORAGE/lanes/tsngen15-sanitizer-targets`): `env -u CMAKE_BUILD_TYPE cmake -S $WORKSPACE_HOME/milan-fpga-management/2026-09-22/tsngen15-author/scripts/embedded-consumer -B $VALIDATION_STORAGE/tmp/a162-tsngen15/build/consumer-cand-embedded-empty -G 'Unix Makefiles' -DTSN_GEN_SOURCE_DIR=$VALIDATION_STORAGE/tmp/a162-tsngen15/src/cand -DCMAKE_EXPORT_COMPILE_COMMANDS=ON`
- `081-cand-embedded-empty-build` (cwd `$VALIDATION_STORAGE/lanes/tsngen15-sanitizer-targets`): `cmake --build $VALIDATION_STORAGE/tmp/a162-tsngen15/build/consumer-cand-embedded-empty -j8`
- `081-cand-embedded-empty-run` (cwd `$VALIDATION_STORAGE/lanes/tsngen15-sanitizer-targets`): `$VALIDATION_STORAGE/tmp/a162-tsngen15/build/consumer-cand-embedded-empty/app`
- `081-cand-embedded-Debug-configure` (cwd `$VALIDATION_STORAGE/lanes/tsngen15-sanitizer-targets`): `env -u CMAKE_BUILD_TYPE cmake -S $WORKSPACE_HOME/milan-fpga-management/2026-09-22/tsngen15-author/scripts/embedded-consumer -B $VALIDATION_STORAGE/tmp/a162-tsngen15/build/consumer-cand-embedded-Debug -G 'Unix Makefiles' -DTSN_GEN_SOURCE_DIR=$VALIDATION_STORAGE/tmp/a162-tsngen15/src/cand -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_BUILD_TYPE=Debug`
- `081-cand-embedded-Debug-build` (cwd `$VALIDATION_STORAGE/lanes/tsngen15-sanitizer-targets`): `cmake --build $VALIDATION_STORAGE/tmp/a162-tsngen15/build/consumer-cand-embedded-Debug -j8`
- `081-cand-embedded-Debug-run` (cwd `$VALIDATION_STORAGE/lanes/tsngen15-sanitizer-targets`): `$VALIDATION_STORAGE/tmp/a162-tsngen15/build/consumer-cand-embedded-Debug/app`
- `082-compare-consumers` (cwd `$WORKSPACE_HOME/milan-fpga-management/2026-09-22/tsngen15-author`): `python3 scripts/compare-configure.py receipts/consumers base cand receipts/compare-consumers-base-vs-candidate.json`
- `083-compare-consumers-neutral` (cwd `$WORKSPACE_HOME/milan-fpga-management/2026-09-22/tsngen15-author`): `python3 scripts/compare-configure.py receipts/consumers base cand receipts/compare-consumers-base-vs-candidate.json`
- `084-recompare-base-candidate` (cwd `$WORKSPACE_HOME/milan-fpga-management/2026-09-22/tsngen15-author`): `python3 scripts/compare-configure.py receipts/configure base cand receipts/compare-base-vs-candidate.json`
- `090-cross-candidate-configure` (cwd `$VALIDATION_STORAGE/lanes/tsngen15-sanitizer-targets`): `env -u CMAKE_BUILD_TYPE cmake -S $VALIDATION_STORAGE/tmp/a162-tsngen15/src/cand -B $VALIDATION_STORAGE/tmp/a162-tsngen15/build/cross-090-cross-candidate -G 'Unix Makefiles' -DCMAKE_SYSTEM_NAME=Linux -DCMAKE_SYSTEM_PROCESSOR=aarch64 -DCMAKE_CXX_COMPILER=aarch64-linux-gnu-g++ -DBUILD_SHARED_LIBS=ON -DCMAKE_EXPORT_COMPILE_COMMANDS=ON`
- `090-cross-candidate-libs` (cwd `$VALIDATION_STORAGE/lanes/tsngen15-sanitizer-targets`): `cmake --build $VALIDATION_STORAGE/tmp/a162-tsngen15/build/cross-090-cross-candidate -j8 --target sanitizer_probe_asan sanitizer_probe_ubsan protocol_parser_asan protocol_parser_ubsan`
- `090-cross-candidate-obj-sanitizer_probe-test_ASAN` (cwd `$VALIDATION_STORAGE/lanes/tsngen15-sanitizer-targets`): `make -C $VALIDATION_STORAGE/tmp/a162-tsngen15/build/cross-090-cross-candidate -f tests/sanitizer/CMakeFiles/sanitizer_probe-test_ASAN.dir/build.make tests/sanitizer/CMakeFiles/sanitizer_probe-test_ASAN.dir/sanitizer_probe_test.cpp.o`
- `090-cross-candidate-obj-sanitizer_probe-test_UBSAN` (cwd `$VALIDATION_STORAGE/lanes/tsngen15-sanitizer-targets`): `make -C $VALIDATION_STORAGE/tmp/a162-tsngen15/build/cross-090-cross-candidate -f tests/sanitizer/CMakeFiles/sanitizer_probe-test_UBSAN.dir/build.make tests/sanitizer/CMakeFiles/sanitizer_probe-test_UBSAN.dir/sanitizer_probe_test.cpp.o`
- `090-cross-candidate-obj-ptp_flags-test_ASAN` (cwd `$VALIDATION_STORAGE/lanes/tsngen15-sanitizer-targets`): `make -C $VALIDATION_STORAGE/tmp/a162-tsngen15/build/cross-090-cross-candidate -f traffic-gen/tests/CMakeFiles/ptp_flags-test_ASAN.dir/build.make traffic-gen/tests/CMakeFiles/ptp_flags-test_ASAN.dir/ptp_flags_test.cpp.o`
- `090-cross-candidate-obj-ptp_flags-test_UBSAN` (cwd `$VALIDATION_STORAGE/lanes/tsngen15-sanitizer-targets`): `make -C $VALIDATION_STORAGE/tmp/a162-tsngen15/build/cross-090-cross-candidate -f traffic-gen/tests/CMakeFiles/ptp_flags-test_UBSAN.dir/build.make traffic-gen/tests/CMakeFiles/ptp_flags-test_UBSAN.dir/ptp_flags_test.cpp.o`
- `090-cross-candidate-exe-link` (cwd `$VALIDATION_STORAGE/lanes/tsngen15-sanitizer-targets`): `cmake --build $VALIDATION_STORAGE/tmp/a162-tsngen15/build/cross-090-cross-candidate -j8 --target sanitizer_probe-test_ASAN`
- `091-cross-M1-base-helper-configure` (cwd `$VALIDATION_STORAGE/lanes/tsngen15-sanitizer-targets`): `env -u CMAKE_BUILD_TYPE cmake -S $VALIDATION_STORAGE/tmp/a162-tsngen15/src/mut-051-M1-base-helper-empty -B $VALIDATION_STORAGE/tmp/a162-tsngen15/build/cross-091-cross-M1-base-helper -G 'Unix Makefiles' -DCMAKE_SYSTEM_NAME=Linux -DCMAKE_SYSTEM_PROCESSOR=aarch64 -DCMAKE_CXX_COMPILER=aarch64-linux-gnu-g++ -DBUILD_SHARED_LIBS=ON -DCMAKE_EXPORT_COMPILE_COMMANDS=ON`
- `091-cross-M1-base-helper-libs` (cwd `$VALIDATION_STORAGE/lanes/tsngen15-sanitizer-targets`): `cmake --build $VALIDATION_STORAGE/tmp/a162-tsngen15/build/cross-091-cross-M1-base-helper -j8 --target sanitizer_probe_asan sanitizer_probe_ubsan protocol_parser_asan protocol_parser_ubsan`
- `091-cross-M1-base-helper-obj-sanitizer_probe-test_ASAN` (cwd `$VALIDATION_STORAGE/lanes/tsngen15-sanitizer-targets`): `make -C $VALIDATION_STORAGE/tmp/a162-tsngen15/build/cross-091-cross-M1-base-helper -f tests/sanitizer/CMakeFiles/sanitizer_probe-test_ASAN.dir/build.make tests/sanitizer/CMakeFiles/sanitizer_probe-test_ASAN.dir/sanitizer_probe_test.cpp.o`
- `091-cross-M1-base-helper-obj-sanitizer_probe-test_UBSAN` (cwd `$VALIDATION_STORAGE/lanes/tsngen15-sanitizer-targets`): `make -C $VALIDATION_STORAGE/tmp/a162-tsngen15/build/cross-091-cross-M1-base-helper -f tests/sanitizer/CMakeFiles/sanitizer_probe-test_UBSAN.dir/build.make tests/sanitizer/CMakeFiles/sanitizer_probe-test_UBSAN.dir/sanitizer_probe_test.cpp.o`
- `091-cross-M1-base-helper-obj-ptp_flags-test_ASAN` (cwd `$VALIDATION_STORAGE/lanes/tsngen15-sanitizer-targets`): `make -C $VALIDATION_STORAGE/tmp/a162-tsngen15/build/cross-091-cross-M1-base-helper -f traffic-gen/tests/CMakeFiles/ptp_flags-test_ASAN.dir/build.make traffic-gen/tests/CMakeFiles/ptp_flags-test_ASAN.dir/ptp_flags_test.cpp.o`
- `091-cross-M1-base-helper-obj-ptp_flags-test_UBSAN` (cwd `$VALIDATION_STORAGE/lanes/tsngen15-sanitizer-targets`): `make -C $VALIDATION_STORAGE/tmp/a162-tsngen15/build/cross-091-cross-M1-base-helper -f traffic-gen/tests/CMakeFiles/ptp_flags-test_UBSAN.dir/build.make traffic-gen/tests/CMakeFiles/ptp_flags-test_UBSAN.dir/ptp_flags_test.cpp.o`
- `091-cross-M1-base-helper-exe-link` (cwd `$VALIDATION_STORAGE/lanes/tsngen15-sanitizer-targets`): `cmake --build $VALIDATION_STORAGE/tmp/a162-tsngen15/build/cross-091-cross-M1-base-helper -j8 --target sanitizer_probe-test_ASAN`
- `085-custom-type-base` (cwd `$VALIDATION_STORAGE/lanes/tsngen15-sanitizer-targets`): `env -u CMAKE_BUILD_TYPE cmake -S $VALIDATION_STORAGE/tmp/a162-tsngen15/src/base -B $VALIDATION_STORAGE/tmp/a162-tsngen15/cfg/custom-base -G 'Unix Makefiles' -DCMAKE_BUILD_TYPE=Profile -DBUILD_SHARED_LIBS=ON -DCMAKE_EXPORT_COMPILE_COMMANDS=ON`
- `085-custom-type-cand` (cwd `$VALIDATION_STORAGE/lanes/tsngen15-sanitizer-targets`): `env -u CMAKE_BUILD_TYPE cmake -S $VALIDATION_STORAGE/tmp/a162-tsngen15/src/cand -B $VALIDATION_STORAGE/tmp/a162-tsngen15/cfg/custom-cand -G 'Unix Makefiles' -DCMAKE_BUILD_TYPE=Profile -DBUILD_SHARED_LIBS=ON -DCMAKE_EXPORT_COMPILE_COMMANDS=ON`
