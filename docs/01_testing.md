# Testing

Testing is central to tsn-gen's development. The project follows Test-Driven
Development (TDD) and provides three layers of verification.

## Layers

| Layer | Framework | Scope |
|-------|-----------|-------|
| Unit tests | GoogleTest | Individual classes and functions |
| Sanitiser runs | ASan / UBSan | Memory and undefined-behaviour safety |
| BDD tests | Behave (Python) | End-to-end observable behaviour |

---

## Unit tests

Each library has a `tests/` subdirectory with GTest-based test executables.
Tests are discovered automatically by CMake via `gtest_discover_tests`.

### Running

```bash
# Build everything (tests are built when ENABLE_PARSER_TESTS=ON, the default)
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j$(nproc)

# Run all tests
ctest --test-dir build --output-on-failure

# Run a specific test binary
./build/parser/tests/object_serializer-test
./build/traffic-gen/tests/traffic_gen-test
```

### Parser unit tests

Located in `parser/tests/`.

| Binary | Test cases | Purpose |
|--------|-----------|---------|
| `gtest_parser-test` | `BasicAssertions` | Sanity check for the GTest setup |
| `protocol_parser-test` | `Version`, `BasePath`, `InvalidPathInt`, `VarCreation`, `VarCreationTooSmall`, `listFiles`, `VerifyDepth`, `DivergingPath`, `ParsingProtocolNameInvalid` | ProtocolParser and Var construction |
| `object_serializer-test` | `ParseSimpleProtocol`, `ParseSimpleProtocolVarCount`, `ParseSimpleProtocolVarName`, `ParseSimpleProtocolVarSize`, `ParseSimpleProtocolVarExpectedValues`, `ParseSimpleProtocolIfaceCount`, `ParseSimpleProtocolIfaceName`, `ParseSimpleProtocolIfaceDirection`, `ParseSimpleProtocolIfaceVarRef` | End-to-end YAML parsing |

### Traffic-gen unit tests

Located in `traffic-gen/tests/`.

| Test case | What it verifies |
|-----------|-----------------|
| `EmptyInterfaceProducesEmptyPacket` | Building from an interface with no var refs returns an empty buffer |
| `SimpleVarProducesOneByte` | A 4-bit var padded to a byte boundary produces a 1-byte packet |
| `SimpleVarValueInExpectedSet` | Over 200 trials, every generated value is in the declared `expected.values` |
| `DeterministicWithSameSeed` | Two `PacketBuilder` instances seeded identically produce identical packets |
| `DifferentSeedsDifferentPackets` | Different seeds produce statistically different output |
| `OpenClose` | `TrafficGenerator::open/close` opens and closes the underlying sender |
| `SendNamedInterface` | Sending a named interface delivers exactly 1 packet to the transport |
| `SendUnknownInterfaceReturnsError` | Returns `TGEN_ERR_IFACE_NOT_FOUND` for an unregistered name |
| `SendAllSendsOnePacketPerInterface` | `sendAll()` produces one packet per interface in the database |
| `SendFilteredInDirection` | `sendFiltered(IN)` sends only IN-direction interfaces |
| `SendFilteredOutDirectionProducesNone` | `sendFiltered(OUT)` sends nothing when all interfaces are IN |
| `SendLoopProducesNPackets` | `sendLoop(name, N)` delivers exactly N packets |
| `SendWithoutOpenReturnsNotOpen` | Calling `send()` before `open()` returns `TGEN_ERR_NOT_OPEN` |
| `VerilatorSenderBeat::FullBeat` | An 8-byte packet becomes 1 beat with `tkeep=0xFF`, `tlast=1` |
| `VerilatorSenderBeat::PartialLastBeat` | A 3-byte remainder produces `tkeep=0x07` |

---

## Sanitiser builds

Every library is compiled in two additional variants:

| Variant | Flag | Detects |
|---------|------|---------|
| `*_asan` | `-fsanitize=address` | Buffer overflows, use-after-free, heap corruption |
| `*_ubsan` | `-fsanitize=undefined` | Signed overflow, null dereference, misaligned access |

Test executables are linked against the instrumented variants so sanitiser
checks run on every `ctest` invocation. A sanitiser error causes the test
binary to exit non-zero, which CTest reports as a failure.

---

## BDD tests

Behaviour tests live in `tests/features/` and are written in Gherkin.
They depend on Python 3 and the Behave framework.

```bash
# Install behave into an isolated virtualenv (first time only)
./tests/run-tests-behave.sh

# Run locally as if running in CI (requires Go for `act`)
go install ...     # install act
./tests/exec-local-test.sh
```

### Pre-commit hook

Install the hook once to run the test suite before every commit:

```bash
./tests/install-pre-commit-hook.sh
```

---

## Test resources

Protocol YAML files used by tests live in `parser/tests/test_resources/`.

| Directory | Purpose |
|-----------|---------|
| `0001_yaml_but_not_a_protocol_description/` | Invalid YAML (missing `service` key) — parser must return an error |
| `0002_max_depth_reached_out/` | 34-level directory nesting — exercises the depth limit (max 16) |
| `0004_invalid_protocol_name/` | `service: 0` (integer, not a string) — type check |
| `0010_simple_protocol_test/` | Minimal valid protocol used by most positive-path tests |

Traffic-gen tests reuse the same resources via the
`TRAFFIC_GEN_TESTS_RES_PATH` CMake define, which points to
`parser/tests/test_resources`.
