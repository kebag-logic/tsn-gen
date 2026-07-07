# Embedding tsn-gen as a library

*For software integrators driving generation and decode from their own C++ application instead of the CLI.*

Everything lives in the C++ namespace `tsn`, headers are included with the
`tsn/` prefix (`#include <tsn/session.h>`), and the CMake package exports
three targets:

| Target | Library | Contents |
|--------|---------|----------|
| `tsn::protocol_parser` | `libprotocol_parser` | YAML parsing, databases, logic-module plumbing |
| `tsn::traffic_gen` | `libtraffic_gen` | Packet build/decode, transports, `tsn::Session` |
| `tsn::protocol_logic` | `libprotocol_logic` | Shipped protocol logic modules |

Most applications only need `tsn::traffic_gen` (it pulls the parser in
transitively).

## Three ways to consume

**1. Installed package — `find_package`**

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)
cmake --install build --prefix /opt/tsn-gen
```

```cmake
# CMAKE_PREFIX_PATH=/opt/tsn-gen
find_package(tsn-gen 0.1 REQUIRED)
target_link_libraries(app PRIVATE tsn::traffic_gen)
```

Pre-1.0 versioning is `SameMinorVersion`: a package built as 0.1.x satisfies
`find_package(tsn-gen 0.1)` but 0.2 will not, since minor bumps may break
API. Note that the package config calls `find_dependency(ryml)` — the public
header `<tsn/protocol.h>` exposes rapidyaml types — so ryml must be findable
from the consumer too.

**2. Embedded — `add_subdirectory`**

```cmake
add_subdirectory(third_party/tsn-gen)
target_link_libraries(app PRIVATE tsn::traffic_gen)
```

When tsn-gen is not the top-level project, unit tests, sanitizer library
variants, and the strict `-Werror` flags are automatically disabled
(`ENABLE_PARSER_TESTS` defaults off for embedders).

**3. Fetched — `FetchContent`**

```cmake
include(FetchContent)
FetchContent_Declare(tsn-gen
    GIT_REPOSITORY <repo-url>
    GIT_TAG        main)
FetchContent_MakeAvailable(tsn-gen)
target_link_libraries(app PRIVATE tsn::traffic_gen)
```

Libraries build static unless `BUILD_SHARED_LIBS=ON`. If you link
`tsn::protocol_logic` statically, mind the registration-TU pitfall described
in [../low-level/logic-modules.md](../low-level/logic-modules.md).

## The `tsn::Session` facade

`tsn::Session` (`<tsn/session.h>`) is the recommended entry point: one object
owns a parsed protocol directory, a seeded PRNG, and the encode/decode
pipelines that `packet_gen` itself is a thin client of.

```cpp
#include <tsn/log.h>
#include <tsn/session.h>

int main()
{
    tsn::setLogLevel(tsn::LogLevel::info);            // diagnostics -> stderr

    tsn::Session session("/usr/share/tsn-gen/protocols");
    if (!session.parse()) return 1;                   // false: nothing loaded

    session.seed(42);                                 // reproducible sequences

    /* ---- single interface ---- */
    const tsn::ProtocolInterface* aecp = session.findInterface(
        "atdecc_aecp_acquire_entity::AECP_ACQUIRE_ENTITY::AECP_ACQUIRE_ENTITY_IF");
    if (!aecp) return 1;

    auto pkt = session.generate(*aecp);               // PacketBuilder::BuiltPacket
    // pkt.bytes  : std::vector<uint8_t>
    // pkt.fields : name -> value pairs, YAML declaration order

    /* ---- multi-layer frame (--stack equivalent) ---- */
    auto frame = session.generateStack({
        session.findInterface("ethernet_mac_frame::ETHERNET_FRAME::ETHERNET_FRAME_IF"),
        session.findInterface("avtp_control_header::AVTP_CONTROL::AVTP_CONTROL_IF"),
        aecp,
    });
    // frame.bytes  : the on-wire frame
    // frame.layers : per-layer name + BuiltPacket slices

    /* ---- command/response chain (--connect equivalent) ---- */
    auto round = session.generateChain({aecp, aecp});

    /* ---- decode ---- */
    auto decoded      = session.decode(*aecp, pkt.bytes);
    auto decodedStack = session.decodeStack(
        {/* same layer list as generateStack */}, frame.bytes);

    /* ---- serialisation: exactly the CLI's NDJSON lines ---- */
    std::string line  = tsn::Session::toJson(pkt);    // {"fields":{...},"hex":"..."}
    std::string sline = tsn::Session::toJson(frame);  // {"hex":"...","layers":[...]}
    std::string cline = tsn::Session::toJson(round);  // [{"iface":...,...}, ...]
    std::string hex   = tsn::Session::toHex(pkt.bytes);

    std::vector<uint8_t> bytes;
    tsn::Session::fromHex(hex, bytes);                // false on malformed hex
    return 0;
}
```

Semantics mirror the CLI (see
[using-the-cli.md](using-the-cli.md)): `generateChain` propagates same-named,
unconstrained fields from stage N to N+1; the interface-list `generateStack`
concatenates independently generated layers; `decodeStack` slices at each
layer's byte offset. `session.parser()` exposes the underlying
`tsn::ProtocolParser` for direct database iteration.

### Logic-driven stacks

For semantically valid frames — derived lengths, demux selectors, corrected
status — load a stack YAML and use the `Stack` overloads, which run each
layer's logic module (link `tsn::protocol_logic` so the modules register):

```cpp
#include <tsn/stack.h>

std::unique_ptr<tsn::Stack> stack;
if (session.loadStack("/usr/share/tsn-gen/stacks/aecp_acquire_entity.yaml",
                      stack).getErrorCode() != tsn::StackBuilderErr::STACK_SUCCESS)
    return 1;

session.seed(42);
auto frame = session.generateStack(*stack);   // onEncode fills subtype, length, ...
auto back  = session.decodeStack(*stack, frame.bytes);   // onDecode + demux check
```

This is the library form of `packet_gen --stack-file`. The modules and their
binding rules are documented in
[../low-level/logic-modules.md](../low-level/logic-modules.md).

## Logging

Library diagnostics go through `tsn::log()` (`<tsn/log.h>`) to **stderr**;
the libraries never write stdout, so your process can keep it
machine-readable.

```cpp
tsn::setLogLevel(tsn::LogLevel::debug);   // quiet | error | warn | info | debug
tsn::setLogLevel("warn");                 // by name; returns false when unknown
tsn::log(tsn::LogLevel::warn) << "field skipped\n";
```

The process-wide default is `warn`.

## Below the facade

For custom pipelines, use the pieces directly — all under `<tsn/...>`:

- `tsn::ProtocolParser` — `parse()` a directory; exposes `Database<Var>`,
  `Database<ProtocolInterface>`, `Database<ProtocolService>`.
- `tsn::PacketBuilder` / `tsn::PacketDecoder` — bit-level encode/decode.
- `tsn::ISender` / `tsn::IReceiver` — transport interfaces with raw-socket,
  pcap, and Verilator implementations; implement them to add a transport.
- `tsn::TrafficGenerator` — parser + builder + sender loop
  (`open`, `send`, `sendAll`, `sendFiltered`, `sendLoop`, `seedRng`).

See [../low-level/architecture.md](../low-level/architecture.md) for how the
pieces fit.

## Installed layout

`cmake --install build --prefix <prefix>` produces:

```
<prefix>/
├── bin/packet_gen                       CLI
├── include/tsn/*.h                      public headers (#include <tsn/...>)
├── lib/
│   ├── libprotocol_parser.so
│   ├── libtraffic_gen.so
│   ├── libprotocol_logic.so
│   └── cmake/tsn-gen/                   tsn-genConfig.cmake + version + targets
└── share/
    ├── man/man1/packet_gen.1
    └── tsn-gen/
        ├── protocols/                   curated YAML corpus (*.yaml)
        └── stacks/                      example stack YAMLs (--stack-file)
```

Point `tsn::Session` (or `packet_gen --yaml-dir`) at
`<prefix>/share/tsn-gen/protocols` for the shipped definitions.

## See also

- CLI equivalents of every call above: [using-the-cli.md](using-the-cli.md)
- YAML schema: [writing-protocols.md](writing-protocols.md)
- Internals: [../low-level/architecture.md](../low-level/architecture.md)
- Documentation map: [../INDEX.md](../INDEX.md)
