# Protocol YAML Reference

Protocol definitions are plain YAML files. One file describes one *service*
(protocol layer). The parser discovers all `.yaml` files under the configured
base directory recursively (up to 16 levels deep).

---

## Top-level structure

```yaml
service: <service_name>        # Required. Must be a non-empty string.

vars:                          # Optional. List of variable definitions.
  - var: <var_name>
    ...

entities:                      # Optional. List of entity definitions.
  - entity: <entity_name>
    ...
```

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
| `size` | Yes | integer | Field width in **bits**. |
| `expected.values` | No | list of integers | When present, the traffic generator picks values exclusively from this list. When absent, any value in `[0, 2^size − 1]` may be generated. |

### Example

```yaml
vars:
  - var: frame_type
    size: 4
    expected:
      values: [0, 1, 2]

  - var: payload_length
    size: 16
    # No expected.values → unconstrained random in [0, 65535]
```

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
        vars:
          var_ref: <var_name>  # single ref
          # — or —
          var_refs:            # multiple refs (order preserved)
            - <var_name_1>
            - <var_name_2>
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
| `dir` | Yes | `in` or `out`. Case-insensitive. |
| `vars` | No | Variable references (see below). |

### Variable references

`vars` may use either form:

```yaml
# Single reference (shorthand)
vars:
  var_ref: simple_var

# Multiple references (order preserved in the packet)
vars:
  var_refs:
    - field_a
    - field_b
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
