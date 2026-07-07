# Architecture

*For contributors who need to know how the pieces fit before changing them.*

All code is in namespace `tsn`; public headers live under each component's
`inc/tsn/` directory and are included as `<tsn/...>`.

## Component graph

```
┌───────────────────────────────────────────────────────────────────────┐
│  Protocol YAML files   (protocols/, or any --yaml-dir tree)           │
│  + stack YAML files    (stack:/layers: shape)                         │
└──────────────────────────────┬────────────────────────────────────────┘
                               │ recursive walk (.yaml, depth ≤ 16)
                               ▼
┌───────────────────────────────────────────────────────────────────────┐
│  protocol_parser  (tsn::protocol_parser)                              │
│                                                                       │
│  ProtocolParser::parse()                                              │
│    └── Protocol (per file, rapidyaml)                                 │
│          ├── getParsedProtocolVars()      → Database<Var>             │
│          ├── getParsedProtocolInterface() → Database<ProtocolInterface>│
│          └── getParsedProtocolService()   → Database<ProtocolService> │
│                                              (carries optional logic:)│
│  Logic subsystem: LogicRegistry · ILogicModule · LayerContext         │
│                   StackBuilder → Stack (bound logic per layer)        │
└──────────────────────────────┬────────────────────────────────────────┘
                               │ databases passed by const-ref
                               ▼
┌───────────────────────────────────────────────────────────────────────┐
│  traffic_gen  (tsn::traffic_gen)                                      │
│                                                                       │
│  PacketBuilder                    PacketDecoder                       │
│    pickValue()  constraints         extractBits()  MSB-first,         │
│    appendBits() MSB-first           layerBytes()   symmetric          │
│                                                                       │
│  Session — facade: generate / generateChain / generateStack /         │
│            decode / decodeStack / toJson / toHex                      │
│                                                                       │
│  Transports                                                           │
│    ISender                          IReceiver                         │
│      ├── RawSocketSender              ├── RawSocketReceiver           │
│      ├── PcapSender                   ├── PcapReceiver                │
│      └── VerilatorSender              └── VerilatorReceiver           │
│          (AxiStreamBeat / UNIX socket, shared with sim/)              │
└──────────────────────────────┬────────────────────────────────────────┘
                               │
                               ▼
┌───────────────────────────────────────────────────────────────────────┐
│  packet_gen (CLI, thin client of Session + transports)                │
│  --interface · --connect · --stack · --stack-file · --decode ·        │
│  --list-interfaces                                                    │
└───────────────────────────────────────────────────────────────────────┘
```

The optional `sim/` tree (SystemC + ns-3 + Verilator co-simulation,
`ENABLE_SYSTEMC_SIM=ON`) reuses `AxiStreamBeat` from
`<tsn/axi_stream_beat.h>` but is otherwise independent; see
[../../sim/README.md](../../sim/README.md).

## Class relationships

### Parser subsystem (`parser/`)

```
Error ◄── DatabaseErr, VarErr, ProtocolErr, ProtocolParserErr, StackBuilderErr

Database<T>                      std::map<std::string, T>; T ∈ {Var,
                                 ProtocolInterface, ProtocolService}
ProtocolParser
    owns ──► Database<Var>                  key "service::var"
    owns ──► Database<ProtocolInterface>    key "service::entity::interface"
    owns ──► Database<ProtocolService>      key "service"
    uses ──► Protocol (per file, transient; ryml::Tree)

Var                 name, size (bits), expectedValues, optional Range{min,max},
                    mask — see <tsn/var.h>
ProtocolInterface   qualifiedName, direction (IN|OUT), ordered varRefs
ProtocolService     name + optional logic-module name (`logic:` key)

LogicRegistry       singleton name → ILogicModule factory (REGISTER_LOGIC)
StackBuilder        stack YAML + Database<ProtocolService> + registry → Stack
Stack               vector<unique_ptr<StackLayer>>; wireAdjacency() links
                    each LayerContext's upper()/lower()
```

### Traffic-gen subsystem (`traffic-gen/`)

```
Error ◄── SenderErr, ReceiverErr, TrafficGeneratorErr

ISender   ◄── RawSocketSender · PcapSender · VerilatorSender
IReceiver ◄── RawSocketReceiver · PcapReceiver · VerilatorReceiver

PacketBuilder
    owns ──► std::mt19937_64
    reads ──► ProtocolInterface (var-ref order), Database<Var>
    returns   BuiltPacket { bytes, fields[name → value] }

PacketDecoder     symmetric: extractBits / layerBytes(iface, varDb)

Session
    owns ──► ProtocolParser + PacketBuilder
    implements the CLI semantics (chain propagation, stack slicing, JSON)

TrafficGenerator
    holds const-ref ──► ProtocolParser
    owns ──► unique_ptr<ISender>, PacketBuilder
    send / sendAll / sendFiltered / sendLoop / seedRng
```

## Data flow: building one packet

```
1. Session::generate(iface)            (CLI: --interface path)
       │
2. PacketBuilder::buildWithFields(iface, varDb)
       for each varRef in iface.getVarRefs():
         ├─ varDb.getElement(ref) → Var        (nullptr → warn + skip)
         ├─ pickValue(var):
         │     expectedValues → uniform pick from list
         │     else range     → uniform pick in [min, max]
         │     else           → uniform in [0, 2^size−1], then &= mask if set
         └─ appendBits(bytes, bitOffset, value, size)   MSB-first
       pad to byte boundary → BuiltPacket { bytes, fields }
       │
3. Output
       ├─ Session::toJson(pkt) → one NDJSON line on stdout
       └─ optional ISender::send(pkt.bytes)
             ├─ RawSocketSender:  sendto(AF_PACKET, sockaddr_ll)
             ├─ PcapSender:       append record to pcap file
             └─ VerilatorSender:  fragment into AxiStreamBeat{tdata,tkeep,tlast}
```

`--connect` wraps step 2 with `buildWithOverrides`: stage N's field map is
passed as overrides to stage N+1, where hard constraints always win over
overrides, and overrides win over free randomness. `--stack` runs step 2 per
layer and concatenates; decode reverses it by slicing at
`PacketDecoder::layerBytes()` offsets.

`--stack-file` runs the logic-driven codec in `tsn::Session`: for each
stack layer it generates field values, loads them into the layer's
`tsn::LayerContext` (name, value, and bit width), runs every module's
`onEncode` — which can read/write fields and reach neighbouring layers to
derive lengths and demux selectors — then packs the wire bytes from the
resulting field list. Decode reverses it: slice each layer, repopulate the
context, run `onDecode`, and check `nextLayer()` against the actual upper
layer. See [logic-modules.md](logic-modules.md).

## Template instantiation pattern

`Database<T>` keeps its implementation out of the public header via explicit
instantiation:

```
parser/inc/tsn/database.h            declaration (installed)
parser/inc/tsn/database.cpp          full template implementation
                                     (never compiled directly)
parser/src/db_var_impl.cpp           #include <tsn/database.cpp>
                                     template class Database<Var>;
parser/src/db_proto_if_impl.cpp      template class Database<ProtocolInterface>;
parser/src/db_proto_service_impl.cpp template class Database<ProtocolService>;
parser/src/db_proto_impl.cpp         (legacy Var instantiation TU)
```

Adding a new `Database<X>` means adding one `db_x_impl.cpp` that includes
`<tsn/database.cpp>` and explicitly instantiates the class.

## Sanitizer build matrix

When `ENABLE_PARSER_TESTS=ON` (default only when tsn-gen is the top-level
project), each library is additionally built in two instrumented variants by
the `generate_test_libs` macro (`cmake/CMakeGenTestingLibraries.cmake`):

```
                  plain      _asan (-fsanitize=address)   _ubsan (-fsanitize=undefined)
protocol_parser     x          x                            x
traffic_gen         x          x                            x
protocol_logic      x          (intentionally not built — mirror
                                generate_test_libs in logic/CMakeLists.txt if needed)
```

`target_link_testlibs` (`cmake/CMakeLinkToTestlibs.cmake`) then produces
three executables per test source — `<name>`, `<name>_ASAN`,
`<name>_UBSAN` — each linked against the matching library variant, and
`gtest_discover_test_wtestlibs` registers all three with CTest. A sanitizer
report makes the binary exit non-zero, which CTest reports as a failure.
Never mix sanitized and unsanitized libraries in one binary — the runtimes
must match.

## Directory layout

```
tsn-gen/
├── CMakeLists.txt            root build + install + CMake package export
├── cmake/                    generate_test_libs / target_link_testlibs /
│                             tsn-genConfig.cmake.in
├── docs/                     this documentation (see ../INDEX.md)
├── external/                 vendored deps (rapidyaml, googletest active;
│                             FTXUI, PcapPlusPlus, protobuf-c, mininet
│                             vendored but not integrated)
├── parser/
│   ├── inc/tsn/              public headers (installed to include/tsn)
│   ├── src/                  protocol.cpp, protocol_parser.cpp, stack*.cpp,
│   │                         logic_registry.cpp, log.cpp, db_*_impl.cpp
│   └── tests/                gtest suites + test_resources/ YAML corpora
├── logic/                    protocol logic modules → tsn::protocol_logic
│   ├── inc/tsn/  src/  tests/
├── protocols/                curated YAML corpus (installed to
│   │                         share/tsn-gen/protocols)
│   ├── application/1722_1/{aecp,adp}/
│   ├── data_link/{1722,ethernet,llc,mac_control}/
│   └── physical/
├── stacks/                   example stack YAMLs for --stack-file (installed
│                             to share/tsn-gen/stacks)
├── traffic-gen/
│   ├── inc/tsn/  src/        builder/decoder/session/transports
│   ├── tools/packet_gen_main.cpp
│   └── tests/                gtest + aecp_behave/ + stack_behave/ BDD suites
├── sim/                      SystemC + ns-3 + Verilator co-simulation (opt-in)
└── tests/                    runner scripts (run-tests.sh, run-tests-behave.sh,
                              exec-local-test.sh) + pre-commit hook installer
```

## Extension points

- **New protocol:** add a YAML file under the corpus — no C++
  ([../mid-level/writing-protocols.md](../mid-level/writing-protocols.md)).
- **New transport:** implement `tsn::ISender` and/or `tsn::IReceiver`
  (`open`/`send`|`receive`/`close`) and wire it where transports are
  selected; nothing else changes.
- **New derived-field / demux behaviour:** write a logic module
  ([logic-modules.md](logic-modules.md)).
- **New tests:** [testing-and-ci.md](testing-and-ci.md).

Documentation map: [../INDEX.md](../INDEX.md).
