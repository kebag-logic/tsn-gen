# tsn-gen documentation

*For everyone: start here to find the right page for your role.*

tsn-gen's documentation is organised in three tiers. Pick the tier that
matches what you want to do, not how much you already know — each page is
self-contained and links onward.

## High level — evaluate the project

For decision-makers and first-time users who want to know what tsn-gen is,
what problems it solves, and how to see it working in five minutes.

| Page | Contents |
|------|----------|
| [high-level/overview.md](high-level/overview.md) | What tsn-gen is, problems it solves, capabilities, honest project status, 5-minute quick start |

## Mid level — use the tools

For test and validation engineers driving the `packet_gen` CLI, protocol
authors writing YAML definitions, and software integrators embedding the
libraries into their own applications.

| Page | Contents |
|------|----------|
| [mid-level/using-the-cli.md](mid-level/using-the-cli.md) | `packet_gen` workflows: generate, seed reproducibility, `--connect` chains, `--stack` / logic-driven `--stack-file` frames, decode, transports, NDJSON piping |
| [mid-level/writing-protocols.md](mid-level/writing-protocols.md) | The protocol YAML schema, a full worked example, naming conventions, DSL limits, stack YAML files |
| [mid-level/embedding-the-library.md](mid-level/embedding-the-library.md) | CMake consumption (`find_package` / `add_subdirectory` / FetchContent), the `tsn::Session` facade, logging, installed layout |

## Low level — change the code

For contributors modifying the parser, traffic generator, logic modules, or
test infrastructure.

| Page | Contents |
|------|----------|
| [low-level/architecture.md](low-level/architecture.md) | Component graph, class relationships, packet build data flow, sanitizer build matrix, template instantiation pattern, directory layout |
| [low-level/logic-modules.md](low-level/logic-modules.md) | `ILogicModule` contract, `LayerContext`, `REGISTER_LOGIC`, StackBuilder binding rules, `bypass-logic` semantics |
| [low-level/testing-and-ci.md](low-level/testing-and-ci.md) | Test pyramid: GTest + sanitizers, Behave BDD suites, runner scripts, GitHub workflows, adding a new test |

## Reference

- `man ./docs/man/packet_gen.1` — the complete `packet_gen` manual page
  ([source](man/packet_gen.1)).

## Legacy numbered docs

The original flat documentation set predates the tier split and the `tsn`
namespace / `<tsn/...>` include-path migration; where they disagree with the
tier pages above, the tier pages follow the current code.

| Document | Description |
|----------|-------------|
| [00_introduction.md](00_introduction.md) | Project goals and design principles |
| [01_testing.md](01_testing.md) | Testing layers: GTest, sanitizers, Behave BDD |
| [02_protocol_parser.md](02_protocol_parser.md) | `ProtocolParser` API and internals |
| [03_protocol_yaml_reference.md](03_protocol_yaml_reference.md) | YAML schema for defining protocols |
| [04_traffic_generator.md](04_traffic_generator.md) | `PacketBuilder`, `ISender`, transports |
| [05_architecture.md](05_architecture.md) | Original component diagram and data flow |

Component READMEs elsewhere in the repository:

- [../README.md](../README.md) — repository front page
- [../logic/README.md](../logic/README.md) — protocol logic modules (full reference; condensed in [low-level/logic-modules.md](low-level/logic-modules.md))
- [../sim/README.md](../sim/README.md) — ns-3 + Verilator SystemC co-simulation
- [../protocols/README.md](../protocols/README.md) — protocol corpus notes
