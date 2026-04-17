# Introduction

TSN-Gen is a C++17 tool for generating, fuzzing, and replaying network
protocol traffic from machine-readable protocol definitions. It targets
Time-Sensitive Networking (TSN) and IEEE 802.1 stacks, but the design is
generic enough to describe any protocol whose variables can be enumerated in
YAML.

## Goals

| # | Goal |
|---|------|
| 1 | Re-use existing protocol developments without recompilation |
| 2 | Simulate or test a whole protocol chain end-to-end |
| 3 | Dissect and harness data at arbitrary stages of the stack |
| 4 | Fuzz harnessed protocol data |
| 5 | Test and understand protocols by monitoring internal variables |
| 6 | Co-simulate with real hardware and virtualised environments |

## Design principles

- **No recompilation.** Protocol definitions live in YAML files that are
  loaded at runtime. Adding a new protocol or interface does not require
  touching C++ sources.
- **Dynamic linking.** Components are built as shared libraries so they can
  be updated independently.
- **Pluggable transport.** The abstract `ISender` interface decouples packet
  generation from delivery. Switching from a real NIC to a Verilator DUT
  is a one-line change.

## High-level architecture

```
  ┌─────────────────────────────────────────────────────────┐
  │                     Protocol YAML files                 │
  └──────────────────────────┬──────────────────────────────┘
                             │ parsed by
                             ▼
  ┌─────────────────────────────────────────────────────────┐
  │                    protocol_parser (lib)                │
  │  ProtocolParser → Database<Var> + Database<ProtocolIf>  │
  └──────────────────────────┬──────────────────────────────┘
                             │ consumed by
                             ▼
  ┌─────────────────────────────────────────────────────────┐
  │                    traffic_gen (lib)                    │
  │   TrafficGenerator                                      │
  │     ├── PacketBuilder  (bit-packs field values)         │
  │     └── ISender                                         │
  │           ├── RawSocketSender  (AF_PACKET / Linux NIC)  │
  │           └── VerilatorSender  (AXI-Stream / UNIX sock) │
  └─────────────────────────────────────────────────────────┘
```

## Repository layout

```
tsn-gen/
├── CMakeLists.txt          Root build configuration
├── cmake/                  Reusable CMake modules
├── docs/                   This documentation
├── external/               Vendored third-party libraries
│   ├── googletest/
│   └── rapidyaml/
├── parser/                 Protocol parser library
│   ├── inc/                Public headers
│   ├── src/                Implementation
│   └── tests/              Unit tests + test resources
├── protocols/              Curated protocol YAML definitions
│   ├── application/
│   ├── data_link/
│   └── physical/
├── traffic-gen/            Traffic generator library
│   ├── inc/
│   ├── src/
│   └── tests/
└── tests/                  Integration / BDD test harness
```

## Building

Requirements: CMake >= 3.31, a C++17-capable compiler (GCC or Clang).

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j$(nproc)
```

To disable unit tests (e.g. for a release build):

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release -DENABLE_PARSER_TESTS=OFF
cmake --build build -j$(nproc)
```
