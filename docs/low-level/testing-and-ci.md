# Testing and CI

*For contributors running the test suites and adding coverage for their changes.*

The project is developed test-first. Verification is layered as a pyramid:
fast GTest unit tests (each also run under ASan and UBSan) at the base,
Behave BDD suites that exercise the real `packet_gen` binary end-to-end on
top, and two GitHub workflows that run both layers on every push and pull
request.

## Layer 1: GTest unit tests + sanitizers

Built when `ENABLE_PARSER_TESTS=ON` (the default when tsn-gen is the
top-level project) and registered with CTest via `gtest_discover_tests`:

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j$(nproc)
ctest --test-dir build --output-on-failure

# a single binary directly
./build/parser/tests/object_serializer-test
./build/traffic-gen/tests/traffic_gen-test_ASAN
```

Current suites:

| Component | Binary | Covers |
|-----------|--------|--------|
| parser | `gtest_parser-test` | GTest setup sanity |
| parser | `protocol_parser-test` | directory walk, depth limit, error paths |
| parser | `object_serializer-test` | end-to-end YAML → Var/Interface databases |
| parser | `logic_registry-test` | REGISTER_LOGIC / LogicRegistry semantics |
| parser | `stack_builder-test` | stack YAML parsing, logic binding rules, bypass |
| traffic-gen | `traffic_gen-test` | PacketBuilder determinism, TrafficGenerator lifecycle, Verilator beat framing |
| traffic-gen | `adp_fuzz-test` | constraint-respecting fuzz generation |
| logic | `ethernet_frame_logic-test` | reference module bound via StackBuilder |
| logic | `stack_codec-test` | logic-driven stack encode/decode golden tests (see below) |

### Sanitizer variants

`generate_test_libs` builds each core library also as `<lib>_asan`
(`-fsanitize=address`) and `<lib>_ubsan` (`-fsanitize=undefined`);
`target_link_testlibs` emits three executables per test — `<name>`,
`<name>_ASAN`, `<name>_UBSAN` — linked against the matching variant, and
`gtest_discover_test_wtestlibs` registers all three. So one `ctest` run
covers plain, ASan, and UBSan. A sanitizer report exits non-zero and fails
the test. The logic library intentionally has no sanitized variant today;
its test links the plain libraries (sanitizer coverage of the shared
machinery comes from the parser suites).

### Stack codec golden tests

`logic/tests/stack_codec_test.cpp` drives the logic-based encode/decode
runtime (`tsn::Session` + the AVTP/AECP modules) against the real
`protocols/` corpus and `stacks/aecp_acquire_entity.yaml`, asserting the
derived fields (EtherType, AVTP subtype, AECP `control_data_length`), the
command-status invariant, and a byte-exact decode round-trip. Its target
links `traffic_gen` in addition to `protocol_logic`, and takes the corpus /
stack paths as the `PROTOCOLS_ROOT_DIR` / `STACKS_ROOT_DIR` compile
definitions.

An optional `stack_codec_tshark` CTest cross-checks a generated frame
against Wireshark's ATDECC dissector (`logic/tests/validate_with_tshark.sh`).
It is registered only when `find_program(tshark)` succeeds, so a host
without tshark skips it instead of failing.

### Test resources

YAML fixtures live in `parser/tests/test_resources/`, numbered by scenario
(`0001_yaml_but_not_a_protocol_description`, `0002_max_depth_reached_out`,
`0010_simple_protocol_test`, `0020_adp_protocol_test`,
`0030_protocol_with_logic`, `0040_stack_test`, ...). They are exposed to the
binaries as compile definitions: `PARSER_TESTS_RES_PATH`,
`TRAFFIC_GEN_TESTS_RES_PATH` (pointing at the parser resources), and
`LOGIC_TESTS_RES_PATH` (`logic/tests/test_resources/`).

## Layer 2: Behave BDD suites

Gherkin features drive the built `packet_gen` binary as a black box. Two
suites live under `traffic-gen/tests/`:

| Suite | Features | Scope |
|-------|----------|-------|
| `aecp_behave/` | 8 feature files | single-layer AECP generation (field constraints, seeds, output formats) plus a layer-stack smoke feature |
| `stack_behave/` | 4 feature files | `--stack` layering, pcap round-trips, decode, DUT interaction |

```bash
# one-time setup
python3 -m venv .venv
.venv/bin/pip install behave

# run (after building, so build/traffic-gen/packet_gen exists)
.venv/bin/behave traffic-gen/tests/aecp_behave/features/
.venv/bin/behave traffic-gen/tests/stack_behave/features/
```

`features/environment.py` resolves paths from the environment:

| Variable | Default | Meaning |
|----------|---------|---------|
| `PACKET_GEN_BIN` | `<repo>/build/traffic-gen/packet_gen` | binary under test |
| `PROTOCOLS_ROOT_DIR` | `<repo>/protocols` | corpus for `--stack` scenarios |
| `AECP_YAML_DIR` | `<repo>/protocols/application/1722_1/aecp` | aecp suite `--yaml-dir` |

Setting `PACKET_GEN_BIN` lets the same features validate an installed or
cross-built binary.

### Environment-dependent tags

Scenarios tagged `@hardware` need `CAP_NET_RAW` (raw-socket injection) and
`@verilator` needs a live DUT socket. The stock runner excludes both:

```bash
./tests/run-tests-behave.sh    # runs both suites with
                               # behave --tags=-hardware --tags=-verilator
```

Run them explicitly on a suitable host:

```bash
sudo .venv/bin/behave --tags=hardware traffic-gen/tests/aecp_behave/features/
```

(A minimal legacy feature also exists at `tests/features/`; the active
suites are the two above.)

## Runner scripts and hooks

| Script | Purpose |
|--------|---------|
| `setup.sh [d]` | configure (`BUILD_SHARED_LIBS=ON`) + build + run unit tests; `d` wipes `build/` first |
| `tests/run-tests.sh [build-dir]` | `ctest --output-on-failure` wrapper; non-zero on failure |
| `tests/run-tests-behave.sh` | creates/uses a venv, installs behave, runs both suites minus `@hardware`/`@verilator` |
| `tests/exec-local-test.sh` | replays the GitHub workflows locally via `act` |
| `tests/run-tests-on-changes.sh` | inotify watch on `parser/` → rebuild + retest |
| `tests/install-pre-commit-hook.sh` | installs a git pre-commit hook that runs `exec-local-test.sh` |

## GitHub workflows

Two workflows in `.github/workflows/`, both triggered on `push` and
`pull_request`, both on `ubuntu-latest`:

- `unit-testing.yml` — checkout + submodules, `./setup.sh d`, then
  `./tests/run-tests.sh`.
- `behave-testing.yml` — same build, then `./tests/run-tests-behave.sh`.

Because the behave runner excludes `@hardware`/`@verilator`, CI stays green
without privileged capabilities or an RTL simulation.

## Adding a new test

**GTest, with sanitizer variants (the default choice).** Add
`my_feature_test.cpp` to the component's `tests/` directory and register it
in that `tests/CMakeLists.txt`:

```cmake
set(MY_FEATURE_SRCS my_feature_test.cpp)

target_link_testlibs(
  my_feature-test        # produces my_feature-test, _ASAN, _UBSAN
  protocol_parser        # base name of the instrumented library to link
  "${MY_FEATURE_SRCS}"
  GTest::gtest_main
  ryml::ryml
)
gtest_discover_test_wtestlibs(my_feature-test)
```

Need a YAML fixture? Add a new numbered directory under
`parser/tests/test_resources/` and reference it through the
`*_TESTS_RES_PATH` define.

**GTest, plain only** (when instrumented variants don't apply, as in
`logic/tests/`): `add_executable` + `target_link_libraries` +
`gtest_discover_tests(name)`.

**Behave.** Add a `.feature` file to the matching suite's `features/`
directory and step definitions under `features/steps/`; reach the binary via
`context.tool` and protocol paths via `context.protocols_dir` /
`context.yaml_dir`. Tag scenarios `@hardware` or `@verilator` when they need
capabilities or a DUT so the default runner and CI skip them.

**Logic-module tests** have their own required patterns (registry test,
bypass test): see [logic-modules.md](logic-modules.md).

Before pushing, run what CI runs:

```bash
./tests/run-tests.sh && ./tests/run-tests-behave.sh
```

Documentation map: [../INDEX.md](../INDEX.md).
