# tsn-gen overview

*For decision-makers and new users evaluating whether tsn-gen fits their problem.*

tsn-gen is a C++17 toolkit that generates, fuzzes, transmits, captures, and
decodes network protocol traffic from machine-readable YAML definitions. It
targets Time-Sensitive Networking (TSN) and the IEEE 1722 / 1722.1 (ATDECC)
stacks, but the engine is protocol-agnostic: any protocol whose fields can be
listed as fixed-width bit fields in YAML works without writing C++.

## Problems it solves

**Protocol debugging.** One YAML file drives both encode and decode. Generate
a frame, capture what a device answers, and decode both sides back to named
fields — the stimulus and the checker share a single source of truth.

**DUT stimulus.** Generated packets go straight onto a Linux NIC
(`AF_PACKET` raw socket), into a Wireshark-compatible pcap file, or into a
Verilator RTL simulation as AXI-Stream beats over a UNIX socket. Switching
transport is one command-line flag; the packet pipeline does not change.

**Hardware co-simulation.** Beyond the socket-based Verilator transport, an
optional SystemC integration (`sim/`) couples ns-3 discrete-event network
simulation with cycle-accurate Verilated RTL under one virtual clock.

**Reproducible fuzzing.** Field values are randomised within the constraints
each YAML declares (fixed values, sets, ranges, bitmasks). The PRNG is a
seeded Mersenne Twister: the same `--seed` always reproduces the same packet
sequence, so a crash found while fuzzing is a one-flag replay.

## Capabilities at a glance

- Runtime-loaded protocol definitions — add or edit a `.yaml` file, no
  recompilation.
- Single-PDU generation, command/response chains with field propagation
  (`--connect`), multi-layer on-wire frames (`--stack`), and logic-driven
  stacks with derived fields filled in (`--stack-file`).
- Symmetric decode: from a hex string, a pcap file, live capture, or a
  Verilator DUT, back to named field values.
- Machine-readable output: newline-delimited JSON (or bare hex) on stdout,
  diagnostics strictly on stderr — safe to pipe into `jq` or Python.
- Usable as a library: CMake package `tsn-gen` with targets
  `tsn::protocol_parser`, `tsn::traffic_gen`, `tsn::protocol_logic`, and a
  single-class facade `tsn::Session`.
- Tested: GTest unit suites with ASan/UBSan variants, plus Behave BDD suites
  that exercise the CLI end to end.

## Project status (honest)

Pre-1.0 (version 0.1.0). What exists today:

- **Protocol coverage:** IEEE 1722.1 AECP (18 AEM and non-AEM message
  types), ADP, the IEEE 1722 AVTP control header, and the Ethernet II MAC
  header ship in `protocols/`. Physical-layer files (MII/GMII/PCS/...) are
  stubs. Other protocols are yours to define in YAML.
- **Generation, decode, transports, CLI:** working and covered by unit and
  BDD tests.
- **Logic runtime: working.** Derived fields (lengths, demux selectors,
  status codes) are filled by per-protocol C++ logic modules during a
  stack encode/decode. The module API (`tsn::ILogicModule`), registry,
  stack builder, and the `tsn::LayerContext` field storage that backs
  module variable access are all in place, with reference modules for
  Ethernet, the AVTP control header, and AECP AEM. Running
  `packet_gen --stack-file stacks/aecp_acquire_entity.yaml` assembles a
  semantically valid IEEE 1722.1 frame (correct EtherType, AVTP subtype,
  and AECP `control_data_length`).

## 5-minute quick start

Requirements: CMake >= 3.21, GCC or Clang with C++17.

```bash
# Build (from the repository root)
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j$(nproc)

# See what the shipped protocol corpus offers
./build/traffic-gen/packet_gen --yaml-dir protocols/ --list-interfaces

# Generate three reproducible IEEE 1722.1 AECP PDUs as NDJSON
./build/traffic-gen/packet_gen \
  --yaml-dir protocols/ \
  --interface atdecc_aecp_acquire_entity::AECP_ACQUIRE_ENTITY::AECP_ACQUIRE_ENTITY_IF \
  --count 3 --seed 42
```

Each output line looks like:

```json
{"fields":{"message_type":0,"status":0,"control_data_length":44,...},"hex":"1402002c..."}
```

## Where to go next

- Drive the CLI in real workflows (pcap capture, chains, stacked frames,
  decode): [../mid-level/using-the-cli.md](../mid-level/using-the-cli.md)
- Describe your own protocol in YAML:
  [../mid-level/writing-protocols.md](../mid-level/writing-protocols.md)
- Embed the libraries in your application:
  [../mid-level/embedding-the-library.md](../mid-level/embedding-the-library.md)
- Understand or change the internals:
  [../low-level/architecture.md](../low-level/architecture.md)
- Full documentation map: [../INDEX.md](../INDEX.md)
