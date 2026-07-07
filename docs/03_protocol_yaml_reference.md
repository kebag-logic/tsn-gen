# Protocol YAML Reference

Protocol definitions are plain YAML files. One file describes one *service*
(protocol layer). The parser discovers all `.yaml` files under the configured
base directory recursively (up to 16 levels deep).

---

## Top-level structure

```yaml
service: <service_name>        # Required. Must be a non-empty string.
logic:   <module_name>         # Optional. Binds a C++ logic module (see below).

vars:                          # Optional. List of variable definitions.
  - var: <var_name>
    ...

entities:                      # Optional. List of entity definitions.
  - entity: <entity_name>
    ...
```

> For a task-oriented walkthrough see the tiered docs
> ([docs/INDEX.md](INDEX.md)); this page is the field-by-field reference.

---

## `service`

The service name becomes the namespace prefix for every var and interface
defined in this file.

```yaml
service: simple_service
```

Rules:
- Must be present and must be a YAML scalar string.
- The value is used verbatim as the first segment of all qualified names.
- Must be unique across all YAML files loaded by a `ProtocolParser` instance.

---

## `vars`

A sequence of variable definitions. Each var describes one field that can
appear in a packet payload.

```yaml
vars:
  - var: <name>            # Short name (within this service)
    size: <bits>           # Field width in bits (positive integer)
    expected:              # Optional value constraints
      values: [v0, v1, ...]
```

### Fields

| Key | Required | Type | Description |
|-----|----------|------|-------------|
| `var` | Yes | string | Short variable name. Qualified as `<service>::<var>`. |
| `size` | Yes | integer | Field width in **bits** (up to 64). |
| `expected.values` | No | list of integers | Generator picks uniformly from this exhaustive list. |
| `expected.value` | No | integer | A single fixed value (shorthand for a one-element `values`). |
| `expected.range` | No | `[min, max]` | Generator picks uniformly from the inclusive range. |
| `expected.mask` | No | `[M]` | Only bits within `M` may be set in the random value. |

Numbers accept decimal or `0x` hex (e.g. `0x22F0`). Values are held as
unsigned 64-bit integers, so a fixed 48-bit MAC or 64-bit entity id is
expressible. A malformed number logs a warning and is treated as 0 rather
than aborting the parse. When no constraint is given, any value in
`[0, 2^size − 1]` may be generated.

### Example

```yaml
vars:
  - var: frame_type
    size: 4
    expected:
      values: [0, 1, 2]

  - var: ethertype
    size: 16
    expected:
      value: 0x22F0            # single fixed value

  - var: descriptor_type
    size: 16
    expected:
      range: [0, 37]           # inclusive range

  - var: flags
    size: 32
    expected:
      mask: [0xC0000000]       # only the top two bits may be set

  - var: payload_length
    size: 16
    # No constraint → unconstrained random in [0, 65535]
```

---

## `logic`

An optional top-level key naming a C++ logic module bound to this service.
The module fills fields the static YAML cannot derive — lengths, checksums,
demux selectors — during the stack runtime (`packet_gen --stack-file`, or
`tsn::Session::generateStack`). When absent, the layer is pure field layout.

```yaml
service: avtp_control_header
logic:   avtp_control          # → AvtpControlLogic, registered in libprotocol_logic
```

See [docs/low-level/logic-modules.md](low-level/logic-modules.md) for the
module contract and how names resolve.

---

## `entities`

A sequence of protocol entities. An entity groups one or more *interfaces*
(input/output points) that belong to the same protocol participant.

```yaml
entities:
  - entity: <entity_name>
    interfaces:
      - interface: <interface_name>
        dir: in | out
        vars:                    # ordered list of field references
          - var_ref: <var_name_1>
          - var_ref: <var_name_2>
```

### Entity fields

| Key | Required | Description |
|-----|----------|-------------|
| `entity` | Yes | Entity name. Qualified as `<service>::<entity>`. |
| `interfaces` | Yes | Non-empty sequence of interface definitions. |

### Interface fields

| Key | Required | Description |
|-----|----------|-------------|
| `interface` | Yes | Interface name. Qualified as `<service>::<entity>::<interface>`. |
| `dir` | Yes | `out` selects the OUT direction; any other value (including `in`) is treated as IN. Match is exact and lower-case. |
| `vars` | No | Ordered list of field references (see below). |

### Variable references

`vars` is a sequence of `var_ref` maps; the order is the on-wire field
order:

```yaml
vars:
  - var_ref: field_a
  - var_ref: field_b
```

Short names are resolved as `<service>::<var_name>` during packet building.
References to vars defined in other services (cross-service references) must
use the fully qualified name:

```yaml
vars:
  var_ref: other_service::other_var
```

If a referenced var cannot be found in the database, the traffic generator
logs a warning and skips the field. The packet is still sent with the
remaining vars.

---

## Complete example

```yaml
service: simple_service

vars:
  - var: simple_var
    size: 4
    expected:
      values: [0, 1, 2]

entities:
  - entity: SIMPLE_SERVICE_ENTITY
    interfaces:
      - interface: SIMPLE_INTERFACE
        dir: in
        vars:
          var_ref: simple_var
```

This produces the following database entries after parsing:

| Database | Key | Value |
|----------|-----|-------|
| `Database<Var>` | `simple_service::simple_var` | size=4, expected={0,1,2} |
| `Database<ProtocolInterface>` | `simple_service::SIMPLE_SERVICE_ENTITY::SIMPLE_INTERFACE` | dir=IN, varRefs=["simple_service::simple_var"] |

---

## Validation rules enforced by the parser

| Check | Error returned |
|-------|---------------|
| File does not contain a `service` key | `PROTOERR_UNEXPECTED` |
| `service` value is not a string (e.g. an integer) | `PROTOERR_UNEXPECTED` |
| A `var` is declared with the same qualified name twice | `DBERR_ENTRY_ALREADY_EXISTS` |
| An `interface` is declared with the same qualified name twice | `DBERR_ENTRY_ALREADY_EXISTS` |
