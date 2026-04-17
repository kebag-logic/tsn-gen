# Architecture

## Component diagram

```
┌──────────────────────────────────────────────────────────────────────┐
│  Protocol YAML files  (protocols/ or parser/tests/test_resources/)   │
└───────────────────────────────┬──────────────────────────────────────┘
                                │  recursive directory walk
                                ▼
┌──────────────────────────────────────────────────────────────────────┐
│                    libprotocol_parser.so                              │
│                                                                       │
│  ProtocolParser                                                       │
│    ├── listProtocolFiles()   (filesystem::recursive_directory_iterator)│
│    └── parseProtocol()                                                │
│           └── Protocol                                                │
│                 ├── checkNodeValidity()                               │
│                 ├── getProtocolName()                                 │
│                 ├── getParsedProtocolVars()   → Database<Var>         │
│                 └── getParsedProtocolInterface() → Database<ProtocIF> │
│                                                                       │
│  Database<Var>                         Database<ProtocolInterface>    │
│    std::map<string, Var>                 std::map<string, ProtocolIf> │
└───────────────────────────────┬──────────────────────────────────────┘
                                │  passed by const-ref
                                ▼
┌──────────────────────────────────────────────────────────────────────┐
│                    libtraffic_gen.so                                  │
│                                                                       │
│  TrafficGenerator                                                     │
│    ├── PacketBuilder                                                  │
│    │     ├── pickValue()   (expectedValues or random [0, 2^size-1])  │
│    │     └── appendBits()  (MSB-first bit packing → []uint8_t)       │
│    └── ISender  ◄──────────────── strategy / dependency injection    │
│          ├── RawSocketSender     AF_PACKET / SOCK_RAW (Linux NIC)    │
│          └── VerilatorSender     AxiStreamBeat / UNIX stream socket  │
└──────────────────────────────────────────────────────────────────────┘
```

---

## Class relationships

### Parser subsystem

```
Error  ◄──── DatabaseErr
       ◄──── VarErr
       ◄──── ProtocolErr
       ◄──── ProtocolParserErr

Database<Var>               // keyed by "service::var"
Database<ProtocolInterface> // keyed by "service::entity::interface"

ProtocolParser
    owns ──► Database<Var>
    owns ──► Database<ProtocolInterface>
    uses ──► Protocol  (per file, not stored)

Protocol
    produces ──► Var objects
    produces ──► ProtocolInterface objects
    uses ──► ryml::Tree  (rapidyaml parse tree)
    uses ──► Entity  (transient, maps entity name → interface names)

Var
    stores: name (string), size (bits), expectedValues ([]int32)

ProtocolInterface
    stores: qualifiedName, direction (IN|OUT), varRefs ([]string)

Entity  (transient, used during YAML parsing only)
    stores: name, list of qualified interface names
```

### Traffic-gen subsystem

```
Error  ◄──── SenderErr
       ◄──── TrafficGeneratorErr

ISender  ◄──── RawSocketSender   (AF_PACKET, Linux-specific)
         ◄──── VerilatorSender   (AXI-Stream / UNIX socket)

TrafficGenerator
    holds const-ref ──► ProtocolParser
    owns ──► unique_ptr<ISender>
    owns ──► PacketBuilder

PacketBuilder
    owns ──► std::mt19937_64  (PRNG)
    uses ──► ProtocolInterface  (var-ref list)
    uses ──► Database<Var>  (field lookup)
```

---

## Data flow: building one packet

```
1. TrafficGenerator::send("svc::ENT::IF")
       │
       └─► getInterfaceDatabase().getElement("svc::ENT::IF")
                 │
                 └─► ProtocolInterface { varRefs: ["svc::v0", "svc::v1"] }

2. PacketBuilder::build(iface, varDb)
       │
       for each varRef in iface.getVarRefs():
         ├─► varDb.getElement(varRef)   → Var { size, expectedValues }
         ├─► pickValue(var)             → uint64_t
         └─► appendBits(buf, offset, value, size)
       │
       └─► pad to byte boundary → []uint8_t

3. ISender::send(packet)
       ├─► RawSocketSender: sendto(sockFd, packet, …, sockaddr_ll)
       └─► VerilatorSender: fragment into AxiStreamBeat, write to UNIX socket
```

---

## Template instantiation pattern

`Database<T>` is a C++ class template. To avoid including the implementation
in every translation unit, the project uses **explicit instantiation**:

```
parser/inc/database.cpp     ← full template implementation (NOT compiled directly)
parser/src/db_var_impl.cpp  ← #include <database.cpp>
                               template class Database<Var>;
parser/src/db_proto_if_impl.cpp ← #include <database.cpp>
                               template class Database<ProtocolInterface>;
```

Each `db_*_impl.cpp` file is compiled three times (plain, `_asan`, `_ubsan`)
to produce the three sanitiser variants of the library.

---

## Sanitiser build matrix

```
                  Plain       ASan         UBSan
                  ─────       ─────        ─────
protocol_parser   ✓           ✓            ✓
traffic_gen       ✓           ✓            ✓

Test executables link against the corresponding sanitised variant:
  protocol_parser-test-asan  → protocol_parser_asan + traffic_gen_asan
  protocol_parser-test-ubsan → protocol_parser_ubsan + traffic_gen_ubsan
```

---

## External dependencies

| Library | Version | Use |
|---------|---------|-----|
| **rapidyaml** | vendored | YAML parsing (`ryml::Tree`, `ryml::parse_in_arena`) |
| **googletest** | vendored | Unit test framework (`TEST`, `TEST_F`, `EXPECT_*`) |

All other dependencies (FTXUI, PcapPlusPlus, protobuf-c, mininet) are
vendored but not yet integrated into the build.

---

## Adding a new protocol

1. Create a YAML file anywhere under the base protocols directory.
2. Follow the schema in [03_protocol_yaml_reference.md](03_protocol_yaml_reference.md).
3. Re-run `ProtocolParser::parse()` (or restart the application).

No C++ changes are needed.

## Adding a new transport

1. Implement `ISender` (`open`, `send`, `close`).
2. Pass a `std::unique_ptr<YourSender>` to `TrafficGenerator`.

No changes to `TrafficGenerator`, `PacketBuilder`, or the parser are needed.
