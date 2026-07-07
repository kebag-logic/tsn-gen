# TSN-Gen

A C++17 toolkit for generating, fuzzing, capturing, and decoding network
protocol traffic from machine-readable YAML definitions. Built for
Time-Sensitive Networking (TSN) and IEEE 802.1 / 1722.1 stacks, but the
design is generic enough to cover any protocol whose fields can be listed in
YAML.

---

## Goals

| # | Goal |
|---|------|
| 1 | Re-use existing protocol definitions without recompilation |
| 2 | Simulate or test a whole protocol chain end-to-end |
| 3 | Dissect and harness data at arbitrary layers of the stack |
| 4 | Fuzz protocol data with reproducible seeds |
| 5 | Test and understand protocols by inspecting internal variables |
| 6 | Co-simulate with real hardware and virtualised RTL environments |

---

## Architecture

```
  ┌──────────────────────────────────────────────────────────────┐
  │             Protocol YAML files  (protocols/)                │
  │   ethernet/mac_frame.yaml · 1722_avtp_control.yaml           │
  │   aecp/aecp_aem_acquire_entity.yaml · adp/…yaml · …          │
  └──────────────────────┬───────────────────────────────────────┘
                         │  recursive directory walk
                         ▼
  ┌──────────────────────────────────────────────────────────────┐
  │              libprotocol_parser.so                           │
  │                                                              │
  │  ProtocolParser                                              │
  │    ├── Database<Var>          keyed "service::var_name"      │
  │    └── Database<ProtocolIf>  keyed "svc::entity::iface"      │
  └──────────────────────┬───────────────────────────────────────┘
                         │  const-ref passed to traffic-gen
              ┌──────────┴──────────┐
              ▼                     ▼
  ┌───────────────────┐   ┌──────────────────────────────────────┐
  │   PacketBuilder   │   │   PacketDecoder                      │
  │                   │   │                                      │
  │  pickValue()      │   │  extractBits()   (MSB-first, sym-    │
  │  appendBits()     │   │                   metric to Builder) │
  │  (MSB-first)      │   │  layerBytes()    (ceil(bits/8))      │
  └────────┬──────────┘   └──────────────────────────────────────┘
           │                        ▲
           │  bytes                 │  bytes
           ▼                        │
  ┌──────────────────────────────────────────────────────────────┐
  │              Transport layer                                 │
  │                                                              │
  │  ISender (generate → DUT)         IReceiver (DUT → verify)   │
  │    ├── RawSocketSender              ├── RawSocketReceiver    │
  │    ├── PcapSender                   ├── PcapReceiver         │
  │    └── VerilatorSender              └── VerilatorReceiver    │
  │        (AXI-Stream / UNIX sock)         (AXI-Stream / UNIX)  │
  └──────────────────────────────────────────────────────────────┘
                         │
                         ▼
  ┌──────────────────────────────────────────────────────────────┐
  │              packet_gen  (CLI)                               │
  │                                                              │
  │  --interface  single PDU generation                          │
  │  --connect    command/response field propagation chain       │
  │  --stack      byte-concatenation of multiple layers          │
  │  --decode     receive and decode frames back to fields       │
  └──────────────────────────────────────────────────────────────┘
```

### Key design decisions

**No recompilation.** Protocol definitions live in YAML files loaded at
runtime. Adding a new protocol or interface field requires only a new or
edited `.yaml` file — no C++ changes.

**Symmetric encode/decode.** `PacketBuilder` packs fields MSB-first into bytes;
`PacketDecoder` reverses the process exactly. The same YAML description drives
both directions, so a DUT stimulus and the expected-output checker share one
source of truth.

**Pluggable transport.** `ISender` and `IReceiver` decouple packet content from
delivery. Switching between a Linux NIC, a pcap file, and a Verilator DUT is a
one-line change — the rest of the pipeline stays identical.

**Layer stacking.** The `--stack` flag concatenates independently generated
layers into a single on-wire frame (e.g. Ethernet 14 B + AVTP 2 B + AECP
variable). `PacketDecoder` slices the frame back at the correct byte offsets
using `layerBytes()`.

---

## Repository layout

```
tsn-gen/
├── CMakeLists.txt                Root build configuration
├── cmake/                        Reusable CMake modules
├── docs/                         Documentation (see below)
├── external/
│   ├── googletest/               Vendored test framework
│   └── rapidyaml/                Vendored YAML parser
├── parser/                       Protocol parser library
│   ├── inc/                      Public headers
│   ├── src/                      Implementation
│   └── tests/                    Unit tests + YAML test resources
├── protocols/                    Curated protocol YAML definitions
│   ├── application/1722_1/
│   │   ├── aecp/                 ATDECC AECP PDUs (20 message types)
│   │   └── adp/                  ATDECC Discovery Protocol
│   ├── data_link/
│   │   ├── 1722/                 AVTP control header
│   │   └── ethernet/             Ethernet II frame header
│   └── physical/                 MAC/PHY interface stubs
└── traffic-gen/                  Traffic generator library + CLI
    ├── inc/                      Public headers
    ├── src/                      Implementation
    ├── tools/
    │   └── packet_gen_main.cpp   packet_gen CLI entry point
    └── tests/
        ├── aecp_behave/          BDD tests for single-layer generation
        └── stack_behave/         BDD tests for layer stacking + decode
```

---

## Building

Requirements: CMake ≥ 3.21, GCC or Clang with C++17 support.

```bash
# Debug build with tests
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j$(nproc)

# Release build (tests disabled)
cmake -B build -DCMAKE_BUILD_TYPE=Release -DENABLE_PARSER_TESTS=OFF
cmake --build build -j$(nproc)
```

The `packet_gen` binary is produced at `build/traffic-gen/packet_gen`.

---

## Using tsn-gen as a library

All library code lives in the `tsn::` namespace and headers are included
with a `tsn/` prefix. Three ways to consume it:

**1. Installed package (`find_package`)**

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)
cmake --install build --prefix /opt/tsn-gen
```

```cmake
find_package(tsn-gen 0.1 REQUIRED)          # CMAKE_PREFIX_PATH=/opt/tsn-gen
target_link_libraries(app tsn::traffic_gen) # or tsn::protocol_parser / tsn::protocol_logic
```

**2. Embedded (`add_subdirectory` / FetchContent)**

```cmake
add_subdirectory(third_party/tsn-gen)       # tests/gtest auto-disabled when embedded
target_link_libraries(app tsn::traffic_gen)
```

**3. From C++ — the `tsn::Session` facade**

```cpp
#include <tsn/session.h>

tsn::Session session("/usr/share/tsn-gen/protocols");
if (!session.parse()) return 1;

session.seed(42);
auto frame = session.generateStack({
    session.findInterface("ethernet_mac_frame::ETHERNET_FRAME::ETHERNET_FRAME_IF"),
    session.findInterface("avtp_control_header::AVTP_CONTROL::AVTP_CONTROL_IF"),
    session.findInterface("atdecc_aecp_acquire_entity::AECP_ACQUIRE_ENTITY::AECP_ACQUIRE_ENTITY_IF"),
});
// frame.bytes = on-wire frame; tsn::Session::toJson(frame) = NDJSON line
```

Library diagnostics go to stderr through `tsn::log()` (`tsn/log.h`,
levels quiet/error/warn/info/debug); stdout stays machine-readable.
The curated `protocols/` corpus installs to `share/tsn-gen/protocols`.

---

## Quick start

```bash
# List all available interfaces in a protocol directory
./build/traffic-gen/packet_gen --yaml-dir protocols/ --list-interfaces

# Generate one AECP ACQUIRE_ENTITY PDU as JSON
./build/traffic-gen/packet_gen \
  --yaml-dir protocols/ \
  --interface atdecc_aecp_acquire_entity::AECP_ACQUIRE_ENTITY::AECP_ACQUIRE_ENTITY_IF

# Assemble a complete IEEE 1722.1 on-wire frame (Ethernet + AVTP + AECP = 55 B)
./build/traffic-gen/packet_gen \
  --yaml-dir protocols/ \
  --stack ethernet_mac_frame::ETHERNET_FRAME::ETHERNET_FRAME_IF\
:avtp_control_header::AVTP_CONTROL::AVTP_CONTROL_IF\
:atdecc_aecp_acquire_entity::AECP_ACQUIRE_ENTITY::AECP_ACQUIRE_ENTITY_IF \
  --seed 42 --count 3

# Capture 100 stacked frames to a pcap file
./build/traffic-gen/packet_gen \
  --yaml-dir protocols/ \
  --stack ethernet_mac_frame::ETHERNET_FRAME::ETHERNET_FRAME_IF\
:avtp_control_header::AVTP_CONTROL::AVTP_CONTROL_IF\
:atdecc_aecp_acquire_entity::AECP_ACQUIRE_ENTITY::AECP_ACQUIRE_ENTITY_IF \
  --count 100 --transport pcap:/tmp/capture.pcap

# Decode that capture back to fields
./build/traffic-gen/packet_gen \
  --yaml-dir protocols/ --decode \
  --stack ethernet_mac_frame::ETHERNET_FRAME::ETHERNET_FRAME_IF\
:avtp_control_header::AVTP_CONTROL::AVTP_CONTROL_IF\
:atdecc_aecp_acquire_entity::AECP_ACQUIRE_ENTITY::AECP_ACQUIRE_ENTITY_IF \
  --transport pcap:/tmp/capture.pcap
```

---

## Testing

```bash
# Run all C++ unit and sanitiser tests
ctest --test-dir build --output-on-failure

# BDD tests — single-layer generation (AECP)
.venv/bin/behave traffic-gen/tests/aecp_behave/features/

# BDD tests — layer stacking, decode, and DUT round-trip
.venv/bin/behave traffic-gen/tests/stack_behave/features/
```

Set up the Python virtual environment once:

```bash
python3 -m venv .venv
.venv/bin/pip install behave
```

---

## Documentation

Start at **[docs/INDEX.md](docs/INDEX.md)** — the documentation is organised
in three tiers by what you want to do:

| Tier | For | Pages |
|------|-----|-------|
| **High level** | evaluating the project | [overview](docs/high-level/overview.md) |
| **Mid level** | using the tools | [CLI](docs/mid-level/using-the-cli.md) · [writing protocols](docs/mid-level/writing-protocols.md) · [embedding the library](docs/mid-level/embedding-the-library.md) |
| **Low level** | changing the code | [architecture](docs/low-level/architecture.md) · [logic modules](docs/low-level/logic-modules.md) · [testing & CI](docs/low-level/testing-and-ci.md) |

The original flat docs (`docs/00`–`05`) remain as reference; where they
disagree with the tier pages, the tier pages follow the current code.

---

## Licence

[MIT](LICENCE.md) — Kebag-Logic, 2025
