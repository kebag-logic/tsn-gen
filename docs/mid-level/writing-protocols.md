# Writing protocol YAML definitions

*For protocol authors describing a wire format so tsn-gen can generate and decode it — no C++ required.*

One YAML file describes one **service** (a protocol layer). The parser walks
the `--yaml-dir` directory recursively (`.yaml` extension only, at most 16
directory levels deep) and loads every file into a shared database, so a file
can live anywhere under the corpus root. Definitions are loaded at runtime —
editing YAML never requires recompiling.

## Top-level structure

```yaml
service: <service_name>   # required — unique namespace prefix for this file
logic: <module_name>      # optional — binds a C++ logic module (see below)

vars:                     # optional — the fields
  - var: <name>
    size: <bits>
    expected: { ... }

entities:                 # optional — groupings that expose interfaces
  - entity: <ENTITY_NAME>
    interfaces:
      - interface: <INTERFACE_NAME>
        dir: in | out
        vars:
          - var_ref: <name>
```

`service` must be a non-empty scalar string and unique across all loaded
files: it becomes the first segment of every qualified name.

## Fields: `vars`

Each entry declares one fixed-width field.

| Key | Required | Meaning |
|-----|----------|---------|
| `var` | yes | Short name; stored as `<service>::<var>` |
| `size` | yes | Width in **bits** (1..64) |
| `expected` | no | Value constraint (below) |

### Value constraints: `expected`

Four constraint forms are parsed; during generation they apply in this
priority order:

| Form | Example | Generator behaviour |
|------|---------|---------------------|
| `value` | `value: 0x22F0` | Always this value (a one-element allowed list) |
| `values` | `values: [0, 1, 2]` | Uniform pick from the list |
| `range` | `range: [0, 37]` | Uniform pick from the inclusive `[min, max]` |
| `mask` | `mask: [0xC0000000]` | Random value ANDed with the mask (only masked bits may be set) |

An exhaustive list (`value`/`values`) beats `range`, which beats `mask`; a
field with no `expected` is free-random in `[0, 2^size - 1]`. Constraints
also shield a field from `--connect` propagation: a hard-constrained field is
never overridden by a previous stage's value.

Numbers may be decimal or `0x` hexadecimal. `mask` is written as a
one-element list.

## Entities and interfaces

An **entity** groups the **interfaces** (generation/decode entry points) of
one protocol participant. Each interface lists the fields it serialises, in
wire order:

- `dir:` is `in` (the DUT receives this data — the usual stimulus direction)
  or `out` (the DUT emits it). Use lowercase; anything other than the exact
  string `out` is treated as `in`.
- `vars:` is a sequence of `- var_ref: <name>` entries. Order in the file is
  bit order on the wire, MSB-first, padded to the next byte boundary.
- A short `var_ref` resolves within the current service. A reference
  containing `::` is kept verbatim, so you can reference another service's
  field: `var_ref: other_service::other_var`.
- A `var_ref` that resolves to nothing is skipped at build time with a
  warning on stderr; the packet is still produced from the remaining fields.

## Naming convention

Every interface is addressed by its fully qualified name:

```
<service>::<ENTITY>::<INTERFACE>
```

Convention in the shipped corpus: lowercase `snake_case` service and var
names, UPPERCASE entity and interface names (e.g.
`atdecc_aecp_acquire_entity::AECP_ACQUIRE_ENTITY::AECP_ACQUIRE_ENTITY_IF`).
Verify what a directory exposes with
`packet_gen --yaml-dir <dir> --list-interfaces`
(see [using-the-cli.md](using-the-cli.md)).

## A full worked example

`protocols/data_link/ethernet/mac_frame.yaml`, the shipped Ethernet II
header, is the canonical minimal file:

```yaml
# Ethernet II frame header (14 bytes).
# Layout (MSB-first): 48b dst_mac | 48b src_mac | 16b ethertype = 112 bits

service: ethernet_mac_frame

vars:
  - var: dst_mac
    size: 48

  - var: src_mac
    size: 48

  - var: ethertype
    size: 16
    expected:
      value: 0x22F0    # IEEE 1722 / 1722.1 ATDECC EtherType

entities:
  - entity: ETHERNET_FRAME
    interfaces:
      - interface: ETHERNET_FRAME_IF
        dir: out
        vars:
          - var_ref: dst_mac
          - var_ref: src_mac
          - var_ref: ethertype
```

After parsing, the databases contain:

| Database | Key | Value |
|----------|-----|-------|
| vars | `ethernet_mac_frame::dst_mac` | size=48, unconstrained |
| vars | `ethernet_mac_frame::src_mac` | size=48, unconstrained |
| vars | `ethernet_mac_frame::ethertype` | size=16, fixed 0x22F0 |
| interfaces | `ethernet_mac_frame::ETHERNET_FRAME::ETHERNET_FRAME_IF` | dir=OUT, 3 var refs |

Try it:

```bash
./build/traffic-gen/packet_gen --yaml-dir protocols/data_link/ethernet \
  --interface ethernet_mac_frame::ETHERNET_FRAME::ETHERNET_FRAME_IF --seed 1
```

The last two bytes of the hex are always `22f0`; the MACs are random.

## What the DSL can and cannot express

The YAML deliberately describes *layout and value sets*, nothing else.

Can express:

- Fixed-width bit fields of 1 to 64 bits, packed MSB-first in declared order.
- Fixed values, allowed-value sets, inclusive ranges, and bitmasks.
- Multiple entities/interfaces per service; cross-service field references.

Cannot express (today):

- **Variable-length fields** — every field has one static `size`; there is
  no length-prefixed payload or TLV support.
- **Conditionals** — no "field B exists only when field A == x", no unions,
  no optional fields.
- **Derived/computed fields** — checksums, CRCs, running sequence numbers,
  and lengths computed from other layers do not belong in YAML. They belong
  to per-protocol C++ **logic modules** which read and write the same named
  vars at encode/decode time; see
  [../low-level/logic-modules.md](../low-level/logic-modules.md).
- Endianness or alignment control — packing is always MSB-first with final
  padding to a byte boundary.

Within a fuzzing campaign these limits are often a feature: everything the
generator can emit is enumerable from the YAML alone.

## Stack YAML files

A second, separate YAML shape describes a *stack* — an ordered set of layers
to bind together, bottom layer first:

```yaml
stack: my_stack            # required stack name
layers:                    # required, non-empty
  - service: ethernet_mac_frame   # must exist in the loaded service DB
    entity:  ETHERNET_FRAME       # optional
  - service: avtp_control_header
    entity:  AVTP_CONTROL
    bypass-logic: true            # optional, default false
```

Per layer, `bypass-logic: true` forces the no-op `PassthroughLogic` module
instead of whatever `logic:` the service YAML declares — the escape hatch
that lets fuzz stacks bypass derived-field fix-ups. The binding rules table
lives in [../low-level/logic-modules.md](../low-level/logic-modules.md).

A stack layer may also name an explicit `interface:` (otherwise the runtime
picks the entity's first interface in key order). Stack YAMLs are consumed
by the library's `tsn::StackBuilder` and driven end-to-end by
`packet_gen --stack-file <yaml>` (and `tsn::Session::generateStack` /
`decodeStack`), which run the per-layer logic modules. The shipped example
is [`stacks/aecp_acquire_entity.yaml`](../../stacks/aecp_acquire_entity.yaml).

## Validating your YAML

The schema above is enforced. Before relying on a new file, check it:

```bash
packet_gen --yaml-dir protocols/ --validate
# 0 errors → conforms; otherwise file:line:col diagnostics on stderr, exit 2
```

`--validate` rejects the things that used to rot the corpus: unknown
top-level keys (foreign dialects like `message:` / `functions:` /
`typedefs:`), a missing `service`, a service with neither `vars` nor
`entities`, out-of-range `size`, a bad `dir`, and malformed `expected`
blocks. The formal contract lives in
[`docs/schema/protocol.schema.json`](../schema/protocol.schema.json) and
[`docs/schema/stack.schema.json`](../schema/stack.schema.json) (usable by
editors and external JSON-Schema tooling). The shipped `protocols/` and
`stacks/` are kept green by the `corpus_validates` CTest.

## See also

- Generating from your new YAML: [using-the-cli.md](using-the-cli.md)
- Parser internals and databases:
  [../low-level/architecture.md](../low-level/architecture.md)
- Documentation map: [../INDEX.md](../INDEX.md)
