# Protocol logic modules

*For contributors adding per-protocol C++ behaviour (derived fields, validation, demux) on top of the YAML wire format.*

A logic module runs alongside a YAML-defined layer inside a built stack. It
never touches bytes: it reads and writes the layer's *named vars* through a
`LayerContext`, while field layout stays owned by the YAML and the
serializer. Layers that are pure layout need no module — the builder binds
the no-op `PassthroughLogic` automatically.

This page condenses [../../logic/README.md](../../logic/README.md) to the
current code. Headers live in the parser: `<tsn/logic_module.h>`,
`<tsn/layer_context.h>`, `<tsn/logic_registry.h>`, `<tsn/stack_builder.h>`,
`<tsn/stack.h>`; shipped modules build into the `tsn::protocol_logic`
library (`logic/`).

## When to write one

- The layer has **derived fields**: length, CRC, timestamp, sequence number.
- The layer has **state**: counters, gap detection, protocol machines.
- The layer **demuxes**: ethertype → next protocol, AVTP subtype → payload
  format.

## The `ILogicModule` contract

From `<tsn/logic_module.h>` (namespace `tsn`):

```cpp
class ILogicModule {
public:
    virtual ~ILogicModule() = default;

    virtual void onEncode(LayerContext& /*ctx*/) {}
    virtual void onDecode(LayerContext& /*ctx*/) {}

    /** Demux hook: service name of the upper layer to dispatch to on
     *  decode, or "" when the next layer is fixed by the stack definition. */
    virtual std::string nextLayer(const LayerContext& /*ctx*/) const
    {
        return {};
    }
};

class PassthroughLogic final : public ILogicModule {};
```

| Hook | When | Job |
|------|------|-----|
| `onEncode` | before the layer's bytes are serialized | fill derived fields the caller left unset; overwrite strictly-computed ones (CRC) |
| `onDecode` | after the layer's bytes are parsed into vars | validate, advance state, snapshot observations |
| `nextLayer` | after `onDecode`, on the receive path | return the upper layer's **service name** (not logic name), or `""` for "use the stack's static next entry" |

Rules that hold across all three:

- Do not throw — nothing catches module exceptions. Log and leave the field
  alone instead.
- Do not write bytes or restructure fields — set vars only.
- Keep `onEncode` idempotent: guard derived writes with "only if unset"
  (`if (!ctx.getValue(name, v) || v == 0) ctx.setValue(name, ...)`).
- Validation failures on decode should be recorded (member flag, stderr
  log), **not** abort processing — fuzzing stacks expect invalid values.
- `nextLayer` is `const`: read state computed in `onDecode`; return `""` for
  unknown demux values so the problem stays visible.

## `LayerContext`

From `<tsn/layer_context.h>`:

```cpp
const std::string& getServiceName() const;
LayerContext* upper() const;        // nullptr at top of stack
LayerContext* lower() const;        // nullptr at bottom
virtual bool getValue(const std::string& name, uint64_t& out) const;
virtual bool setValue(const std::string& name, uint64_t value);
```

- Use **short (unqualified) var names**: `ctx.getValue("ethertype", v)`.
  The context is scoped to its layer.
- Reach adjacent layers through the adjacency pointers:
  `ctx.lower()->getValue("src_mac", v)`. The pointers are stable for the
  stack's lifetime (layers are stored as `unique_ptr<StackLayer>`), but only
  after `Stack::wireAdjacency()` — so capture them on first
  `onEncode`/`onDecode`, never in the module constructor.
- `uint64_t` is the only value type; fields up to 64 bits fit.

`LayerContext::getValue`/`setValue` operate on the layer's real, ordered
field storage that the runtime loads before `onEncode`/`onDecode`.
`getValue` returns `false` for an unknown field; `setValue` only writes
fields the YAML declared (a module cannot invent a wire field). For length
derivation the context also exposes `byteSize()`, `bytesAfter(field)`, and
`bytesAbove()`.

## Registration: `REGISTER_LOGIC`

```cpp
// in a .cpp that links into the final binary
REGISTER_LOGIC("ethernet_mac_frame", EthernetFrameLogic)
```

The macro (from `<tsn/logic_registry.h>`) expands to a file-scope static
that calls `tsn::LogicRegistry::instance().add(name, factory)` at static
initialization. Consequences:

- The **name** is the string a service YAML's `logic:` key refers to —
  convention: match the primary service name. Service name, logic name, and
  class name are three different identifiers that merely tend to coincide.
- The class must be default-constructible.
- Duplicates are rejected: first registration wins, the second `add` returns
  `false`, silently. Keep names unique.
- Registration only happens if the translation unit is linked. Shared
  library: just link. **Static library: the linker drops the TU** unless you
  force it (`-Wl,--whole-archive liblogic.a -Wl,--no-whole-archive`). Always
  pair a new module with a registry test (below).

## Binding: stack YAML and the rules table

A service YAML opts into logic with one key:

```yaml
service: ethernet_mac_frame
logic: ethernet_mac_frame        # string key into LogicRegistry
vars: [...]
entities: [...]
```

A stack YAML (`tsn::StackBuilder::build(path, outStack)`) lists layers
bottom-up and may bypass per layer:

```yaml
stack: my_stack
layers:
  - service: ethernet_mac_frame   # must exist in Database<ProtocolService>
    entity:  ETHERNET_FRAME       # optional
    bypass-logic: true            # optional, default false
```

Resolution per layer:

| service declares `logic:` | `bypass-logic:` | name in registry? | Bound module | Builder result |
|---------------------------|-----------------|-------------------|--------------|----------------|
| no  | false / absent | — | `PassthroughLogic` | OK |
| no  | true           | — | `PassthroughLogic` | OK |
| yes | false / absent | yes | registered class | OK |
| yes | false / absent | no  | — | `STACK_ERR_UNKNOWN_LOGIC` (hard failure, no fallback) |
| yes | true           | yes | `PassthroughLogic` (registry not consulted) | OK |
| yes | true           | no  | `PassthroughLogic` (registry not consulted) | OK — **fuzzing path** |

The last row matters: a fuzz stack can reference services whose logic
modules are not linked into the binary, because `bypass-logic: true`
short-circuits the registry lookup entirely.

**`bypass-logic` does not bypass field layout.** YAML constraints like
`expected.value: 0x22F0` still apply at serializer level; bypass only
removes the module's derived-field and validation behaviour.

## Lifecycle

```
StackBuilder::build(path, outStack)
 ├─ per layer: resolve service → bind module (table above)
 │             construct StackLayer{ serviceName, entityName, logicName,
 │                                   bypassLogic, logic, context }
 └─ Stack::wireAdjacency()   // set upper/lower on every LayerContext
```

One module instance per stack build — not per packet. Member state persists
across every packet through that stack, which is exactly where sequence
counters and seen-sets belong. No statics, no globals: two stacks must not
interfere. Modules die with the owning `Stack`; there is no shutdown hook.

## Reference module

The shipped example is `tsn::EthernetFrameLogic`
(`logic/inc/tsn/ethernet_frame_logic.h`, registered as
`"ethernet_mac_frame"`):

- `onEncode` writes `ethertype = 0x22F0` only when unset (idempotent).
- `onDecode` snapshots the last ethertype seen and counts calls, so tests
  can observe activity via `getLastEthertypeSeen()` / `getEncodeCalls()` /
  `getDecodeCalls()`.
- `nextLayer` returns the bound upper layer's service (the concrete demux
  target in a static stack), falling back to the 1722 AVTP service when the
  module is driven standalone with no upper layer.

Two more reference modules ship in `libprotocol_logic`:

- **`AvtpControlLogic`** (`logic: avtp_control`) sets the AVTP `subtype`
  from the upper service — `0xFB` for AECP, `0xFA` for ADP — and forces the
  control invariants `sv=0` / `version=0`.
- **`AecpAemLogic`** (`logic: aecp_aem`) derives `control_data_length` as
  `bytesAfter("control_data_length") + bytesAbove()` and forces `status=0`
  on commands (even `message_type`), leaving responses untouched.

The end-to-end behaviour is covered by
`logic/tests/stack_codec_test.cpp`, which drives the shipped
`stacks/aecp_acquire_entity.yaml` through `tsn::Session`.

## Testing a module

- **Isolation:** instantiate the module with a plain `LayerContext` and
  drive the hooks; assert on the module's own accessors.
- **In a stack:** build via `StackBuilder`, then
  `dynamic_cast<MyLogic*>(stack->layers()[n]->logic.get())` — the supported
  introspection primitive.
- **Registry test (always add one):**
  `EXPECT_TRUE(tsn::LogicRegistry::instance().has("my_name"));` — catches
  the dropped-TU linker failure early.
- **Bypass test:** build a stack with `bypass-logic: true` on your layer and
  assert `PassthroughLogic` got bound.

Existing examples: `parser/tests/logic_registry_test.cpp`,
`parser/tests/stack_builder_test.cpp`,
`logic/tests/ethernet_frame_logic_test.cpp`. How to wire new tests into the
build: [testing-and-ci.md](testing-and-ci.md).

Documentation map: [../INDEX.md](../INDEX.md).
