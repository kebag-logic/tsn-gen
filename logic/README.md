# Protocol logic modules

Per-protocol C++ modules that run alongside the YAML-defined wire
format. A module fills derived fields on encode, validates and drives
demux on decode, and leaves pure field layout to the parser.

The top of this document is a quick-start. The sections below are the
authoritative reference — contract, lifecycle, worked examples, and
gotchas.

---

## Quick start

### When to add one

- Layer has derived fields (length, CRC, sequence, timestamp).
- Layer has state (sequence counters, ADP/MSRP machines).
- Layer does demux (ethertype -> next protocol, subtype -> AAF vs CVF).

Layers that are just field layout (most MAC-ish headers) need no
module — the StackBuilder binds `PassthroughLogic` automatically.

### Writing a module

1. Subclass `ILogicModule` (`parser/inc/logic_module.h`), override any
   of `onEncode`, `onDecode`, `nextLayer`.
2. Register it with `REGISTER_LOGIC("<logic-name>", MyLogic)` in a
   `.cpp` that ends up in this shared library.
3. In the service's YAML, add `logic: <logic-name>` at the top level.

Minimal skeleton:

```cpp
#include <logic_module.h>
#include <logic_registry.h>
#include <layer_context.h>

class MyLogic final : public ILogicModule {
public:
    void onEncode(LayerContext& ctx) override { /* ... */ }
    void onDecode(LayerContext& ctx) override { /* ... */ }
    std::string nextLayer(const LayerContext&) const override {
        return "upper_service_name";
    }
};

REGISTER_LOGIC("my_service", MyLogic)
```

### Bypassing logic (fuzzing)

Per-layer escape hatch in the stack YAML:

```yaml
stack: my_stack_fuzz
layers:
  - service: ethernet_mac_frame
    entity:  ETHERNET_FRAME
    bypass-logic: true          # binds PassthroughLogic here
  - service: 1722_avtp_common_stream
    entity:  AVTP_STREAM
```

With `bypass-logic: true` the StackBuilder does not consult the
registry for that layer. The service's declared module is ignored,
and derived fields stay whatever the caller wrote. Only byte-level
field layout still applies.

---

## Reference

### The mental model

A logic module sits between a protocol's YAML-defined field layout
and the runtime that builds/parses packets. It does not touch bytes.
It reads and writes *named vars* — the same vars the YAML declared —
through a `LayerContext` the Stack hands it.

Three questions a module can answer:

| When                          | Question                                             | Method                                   |
| ----------------------------- | ---------------------------------------------------- | ---------------------------------------- |
| Before bytes are serialized   | "Fill in anything I derive from other fields or state." | `onEncode(LayerContext&)`             |
| After bytes are parsed        | "Validate, advance state, record what I saw."        | `onDecode(LayerContext&)`                |
| During decode demux           | "Which upper-layer service handles the rest?"        | `nextLayer(const LayerContext&) const`   |

A layer's module is bound at stack build time and lives for the life
of the `Stack`. Per-packet, the runtime calls `onEncode` (tx) or
`onDecode` then `nextLayer` (rx) on the module for each layer.

### The `ILogicModule` contract

Defined in `parser/inc/logic_module.h`:

```cpp
class ILogicModule {
public:
    virtual ~ILogicModule() = default;
    virtual void onEncode(LayerContext& /*ctx*/) {}
    virtual void onDecode(LayerContext& /*ctx*/) {}
    virtual std::string nextLayer(const LayerContext& /*ctx*/) const { return {}; }
};
```

#### `onEncode`

Called after the caller populated any fields they care about, before
the layer's bytes are serialized. Your job:

- Fill derived fields the caller did not set (length, CRC, timestamp,
  sequence counter).
- Overwrite fields you consider strictly computed (e.g. CRC — don't
  trust a caller-provided one).
- Touch adjacent layers when the field you own depends on them
  (`ctx.upper()`, `ctx.lower()`).

What you must **not** do:

- Write bytes directly — you don't have them yet. Set vars; the
  serializer converts.
- Throw. The current error surface is not prepared for exceptions in
  modules. If you detect an unrecoverable inconsistency, log and leave
  the field alone; tests will catch it.
- Resize or restructure fields — field layout is owned by the YAML.

Keep `onEncode` idempotent — guard derived writes with "only if unset"
where that makes sense. `EthernetFrameLogic::onEncode` shows the
pattern: `if (!ctx.getValue(...) || current == 0) ctx.setValue(...)`.

#### `onDecode`

Called after the layer's bytes have been parsed into vars, before any
upper-layer parsing. Your job:

- Validate — compare `ethertype`, `version`, CRC, range constraints
  not already covered by `Var::expected`.
- Drive state machines (sequence gaps, out-of-order detection).
- Snapshot what you saw, if tests or peers need to read it back.

Validation failures in v1 should:

- Set an internal flag the test can read (see
  `EthernetFrameLogic::mLastEthertype`).
- Log to `std::cerr` for humans.
- **Not** abort packet processing — the runtime keeps walking the
  stack even on "invalid" layers, because fuzzing stacks *expect*
  invalid values. When real rejection semantics are needed, that's a
  new API; don't build it inline.

#### `nextLayer`

Called after `onDecode`, on the receive path. Return:

- `""` (default) — "I don't dispatch. The stack YAML's next entry is
  the upper layer."
- A string — the fully-qualified **service name** (not logic name)
  that handles the remaining bytes. The runtime looks it up in
  `Database<ProtocolService>` via the same name the service's YAML
  declared.

This method is `const` — no state changes. If you need state to
decide, compute it in `onDecode`; `nextLayer` just reads it.

Patterns:

| Situation                              | What to return                                             |
| -------------------------------------- | ---------------------------------------------------------- |
| Fixed chain (no demux at this layer)   | `""`                                                       |
| Demux by one field                     | Look it up, return the mapped service name                 |
| Demux with unknown value               | Return `""` so the static upper layer takes over, or a "sink" service you register |

### `LayerContext` — what's there, what isn't

Defined in `parser/inc/layer_context.h`. What the module sees:

```cpp
const std::string& getServiceName() const;
LayerContext* upper() const;                   // nullptr at top
LayerContext* lower() const;                   // nullptr at bottom
virtual bool getValue(const std::string& name, uint64_t& out) const;
virtual bool setValue(const std::string& name, uint64_t value);
```

**Critical v1 caveat:** `getValue`/`setValue` are stubs that return
`false`. They will be overridden by a Stack-runtime subclass once the
serializer lands. Write your module as if they work (they will) —
your tests against a plain `LayerContext` will just see `false` and
that is fine. `EthernetFrameLogic` calls them today and its behavior
degrades gracefully to "no-op with counters" under the stubs.

**Naming:** `getValue("ethertype", ...)` uses the short (unqualified)
var name. The context is scoped to the layer; it will prepend the
service name internally when the accessors are backed. Don't pass a
fully-qualified `service::ethertype` — that's a layering leak.

**Adjacency pointers are stable** for the life of the stack. The
stack stores `unique_ptr<StackLayer>` specifically so
`&layer->context` never moves. You can cache a raw `LayerContext*`
inside your module.

**Reaching peers:** `ctx.lower()->getValue("src_mac", out)` is the
supported way to read a field from another layer. Don't try to cast
`LayerContext*` to something protocol-specific — there's no such
thing yet, and there shouldn't be.

### Lifecycle

```
StackBuilder::build(path, out)
 ├─ for each layer in stack YAML:
 │    - look up ProtocolService in Database
 │    - if bypass-logic OR !hasLogic → make_unique<PassthroughLogic>()
 │    - else                         → registry.create(logicName)
 │    - construct LayerContext(serviceName)
 │    - push StackLayer{ logic, context, ... } into Stack
 └─ Stack::wireAdjacency()   // set upper/lower pointers
```

Each module instance is created **once per stack build** (not per
packet). State declared as non-static members persists across every
packet that goes through that stack. That is why `EthernetFrameLogic`
stores `mLastEthertype` and counters as normal members — they
accumulate for the life of the `Stack`.

Two implications:

1. Sequence counters, rate limits, seen-set caches go in member
   fields. No statics, no globals — two stacks must not interfere.
2. Cross-packet state is not cross-stack. If you tear down and
   rebuild the stack, your counters reset.

Modules are destroyed when the owning `Stack` is destroyed. No
explicit `shutdown()` hook. If one becomes necessary later, add it to
`ILogicModule` with a default empty body — that is backwards
compatible.

### YAML wiring — full binding table

Service YAML gains one optional key:

```yaml
service: ethernet_mac_frame
logic: ethernet_mac_frame      # string key into LogicRegistry
vars: [...]
entities: [...]
```

Stack YAML:

```yaml
stack: <stack_name>             # required
layers:                         # required, non-empty sequence
  - service: <service_name>     # required, must exist in the service DB
    entity:  <entity_name>      # optional, echoed into StackLayer
    bypass-logic: true          # optional, defaults to false
```

| service declares `logic:` | `bypass-logic:` | registry has the name? | Bound module                              | Builder result              |
| ------------------------- | --------------- | ---------------------- | ----------------------------------------- | --------------------------- |
| no                        | false / absent  | —                      | `PassthroughLogic`                        | OK                          |
| no                        | true            | —                      | `PassthroughLogic`                        | OK                          |
| yes                       | false / absent  | yes                    | registered class                          | OK                          |
| yes                       | false / absent  | no                     | —                                         | `STACK_ERR_UNKNOWN_LOGIC`   |
| yes                       | true            | yes                    | `PassthroughLogic` (registry not consulted) | OK                        |
| yes                       | true            | no                     | `PassthroughLogic` (registry not consulted) | OK — **fuzzing path**     |

The last row is the important one: a fuzz stack can reference services
whose logic modules are not in the test binary, and the builder still
succeeds because `bypass-logic: true` short-circuits the registry
lookup.

### Worked examples

#### Passthrough — already written

```cpp
class PassthroughLogic final : public ILogicModule {};
```

Inherits all default no-ops. Use as a reference for "zero state,
zero behavior." This is what the Stack binds when there is nothing
to do.

#### Constant field fill (Ethernet)

```cpp
class EthernetFrameLogic final : public ILogicModule {
public:
    static constexpr uint16_t kEthertype1722 = 0x22F0;

    void onEncode(LayerContext& ctx) override {
        uint64_t current = 0;
        if (!ctx.getValue("ethertype", current) || current == 0) {
            ctx.setValue("ethertype", kEthertype1722);
        }
    }

    void onDecode(LayerContext& ctx) override {
        ctx.getValue("ethertype", mLastEthertype);
    }

    std::string nextLayer(const LayerContext& ctx) const override {
        uint64_t eth = 0;
        if (ctx.getValue("ethertype", eth) && eth == kEthertype1722) {
            return "1722_avtp_common_stream";
        }
        return "1722_avtp_common_stream";   // v1 fallback
    }

private:
    uint64_t mLastEthertype = 0;
};

REGISTER_LOGIC("ethernet_mac_frame", EthernetFrameLogic)
```

Key points:

- Idempotent encode: only writes if unset.
- Decode snapshots for observability — tests read the member.
- Demux falls back to a known-good upper service while `getValue`
  stubs return false. When the serializer lands, the fallback
  becomes the explicit "unknown ethertype" branch and the 0x22F0
  branch is the happy path.

#### Stateful counter (sequence number)

```cpp
class SequenceCounterLogic final : public ILogicModule {
public:
    void onEncode(LayerContext& ctx) override {
        uint64_t caller = 0;
        if (!ctx.getValue("sequence_num", caller)) {
            ctx.setValue("sequence_num", mNextSeq++);
        } else {
            mNextSeq = caller + 1;   // caller forced a value, resync
        }
    }

    void onDecode(LayerContext& ctx) override {
        uint64_t seen = 0;
        if (ctx.getValue("sequence_num", seen)) {
            if (mLastSeen.has_value() && seen != *mLastSeen + 1) {
                ++mGapCount;
            }
            mLastSeen = seen;
        }
    }

    uint32_t getGapCount() const { return mGapCount; }

private:
    uint8_t mNextSeq = 0;
    std::optional<uint64_t> mLastSeen;
    uint32_t mGapCount = 0;
};

REGISTER_LOGIC("1722_sequence_counter", SequenceCounterLogic)
```

Notes:

- All state is a member — per-stack, not global.
- "Caller forced a value" branch: cooperate with a fuzzer that sets
  `sequence_num` explicitly instead of fighting it. This is the right
  default for most counters.
- Introspection accessor (`getGapCount`) is a non-virtual member.
  Tests reach it by `dynamic_cast<SequenceCounterLogic*>(layer->logic.get())`.

#### Demux with a table

```cpp
class AvtpCommonStreamLogic final : public ILogicModule {
public:
    void onDecode(LayerContext& ctx) override {
        ctx.getValue("subtype", mSubtype);
    }

    std::string nextLayer(const LayerContext&) const override {
        switch (mSubtype) {
            case 0x02: return "1722_aaf";
            case 0x03: return "1722_cvf";
            case 0x04: return "1722_crf";
            default:   return "";   // let the static stack take it
        }
    }

private:
    uint64_t mSubtype = 0;
};

REGISTER_LOGIC("1722_avtp_common_stream", AvtpCommonStreamLogic)
```

Design choices worth calling out:

1. `mSubtype` is stored as state rather than re-read in `nextLayer`.
   `nextLayer` is `const`, and you already read the field in
   `onDecode`. Avoids a second lookup.
2. Unknown subtypes return `""` not a default. Unknown demux should
   be visible to the stack, not silently routed.

### Testing patterns

**Module in isolation — no stack:**

```cpp
TEST(MyLogic, EncodesConstantWhenUnset) {
    MyLogic m;
    LayerContext ctx("my_service");
    m.onEncode(ctx);
    // With LayerContext v1 stubs, verify via member accessors
    // or cross-check that the counters advanced.
    EXPECT_EQ(m.getEncodeCalls(), 1);
}
```

Use this to test state transitions, idempotence, and branching logic
without pulling in the parser/stack.

**Module inside a stack — end to end:**

```cpp
TEST(MyLogic, BoundByStackBuilder) {
    ProtocolParser parser(protoDir);
    parser.parse();
    StackBuilder builder(parser.getServiceDatabase());
    std::unique_ptr<Stack> stack;
    ASSERT_EQ(builder.build(stackYaml, stack).getErrorCode(),
              StackBuilderErr::STACK_SUCCESS);
    auto* m = dynamic_cast<MyLogic*>(stack->layers()[N]->logic.get());
    ASSERT_NE(m, nullptr);
    // exercise via the stack, then read back through m
}
```

`dynamic_cast` is the introspection primitive. Pattern lives in
`logic/tests/ethernet_frame_logic_test.cpp`.

**The bypass test:** any module you add should have a corresponding
test that builds a stack with `bypass-logic: true` on that layer and
asserts `PassthroughLogic` got bound in its place. Catches regressions
where someone refactors the bypass path.

**Registry test:**

```cpp
TEST(MyLogic, Registered) {
    EXPECT_TRUE(LogicRegistry::instance().has("my_service"));
}
```

Detects the static-library-drop-the-TU linker bug early. Always
include it when you add a new module.

### Error surface

- Module code that throws: not caught anywhere. Tests abort, real
  runs crash. Don't throw.
- `setValue` on an unknown var in v1: returns `false`, otherwise
  silent. Log if it matters to your module.
- Registry name collision: `REGISTER_LOGIC` evaluates to `false`,
  first registration wins, no warning. Detect with the "Registered"
  test plus a uniqueness check if you have many modules.
- Stack YAML referencing your logic name when the library isn't
  linked: `STACK_ERR_UNKNOWN_LOGIC`, hard failure at build. The
  builder does not fall back to Passthrough. If you want fallback,
  opt in with `bypass-logic: true`.

### The registry

`REGISTER_LOGIC(name, Class)` expands to a file-scope
`static const bool _logic_reg_Class = LogicRegistry::instance().add(...)`
(`parser/inc/logic_registry.h`). Two consequences:

- Registration happens at **static initialization**, before `main`.
  You can't pass runtime values.
- Registration happens only if the TU is linked into the final
  binary. A static library will drop the TU unless you force-load it.

Rules:

- `name` is the string that goes in service-YAML `logic:`. Not the
  class name, not the service name — the **module** name. Convention:
  match the primary service it is used by
  (`ethernet_mac_frame` for `EthernetFrameLogic`), which keeps the
  YAML unsurprising.
- `Class` must be constructible with no arguments. If you need init
  args, store them in a registered `static` config struct the class
  reads at construction.
- Register exactly once per process. `LogicRegistry::add` rejects
  duplicates; the second call returns `false` and the first
  registration wins.

Linkage pitfalls:

- **Shared (recommended)** — just link and go.
  `LogicRegistry::has("your_name")` returns true after first load.
- **Static** — add `-Wl,--whole-archive libmy_logic.a -Wl,--no-whole-archive`
  or the TU with the registration gets dropped and your module is
  silently absent. Test this by asserting `LogicRegistry::has(...)`
  in a first-run fixture.
- **Mixing ASAN / UBSAN builds** — if your tests sanitizer-link a
  sanitized protocol_logic, it must be built against a sanitized
  protocol_parser; sanitizer runtimes must match or ODR violations
  fire. The sanitized variant of the logic lib is intentionally not
  built today; if you need it, mirror `generate_test_libs` in
  `logic/CMakeLists.txt`.

### Gotchas

1. **Static init order across TUs is unspecified.** Don't reach into
   other registries / globals from your module's constructor. Build
   init lazily on first `onEncode`/`onDecode` if you truly need other
   statics.
2. **Service name, logic name, and class name are three different
   things.** In the reference they happen to match
   (`ethernet_mac_frame`); that's a convention, not a requirement.
3. **`LayerContext` is moved-into-place by `Stack`.** Don't capture
   its address during module construction. Capture on first
   `onEncode`/`onDecode` (the Stack has called `wireAdjacency` by
   then) if you must.
4. **`nextLayer` returning the wrong name is silent** until the
   runtime tries to dispatch to it. The builder has no way to know
   demux targets in advance. A test that walks all possible demux
   values and asserts each mapped name exists in the service DB is
   cheap insurance.
5. **`bypass-logic: true` does not bypass field layout.** Your YAML's
   `expected.values: [0x22F0]` constraint still applies because that's
   serializer-level, not logic-level. Raw fuzzing below serializer
   range checks needs a different switch (not yet implemented — flag
   it when you need it).
6. **Module lifetime = stack lifetime.** Don't expect destructors to
   run mid-packet. Don't stash pointers to a module past the stack's
   lifetime.

### Forward compatibility

Write modules today against the contract above — it is the stable
surface. What will change underneath:

- **`LayerContext::getValue`/`setValue` will start returning `true`**
  and actually touching var storage. Your module code won't change;
  behavior goes from "no-op with counters" to real derived-field work.
- **Typed accessors** (`getValueI32`, `getValueBytes` for MAC
  addresses) will be added. `uint64_t` is the v1 only-type. Don't
  paper over this — if you need a 48-bit field today, note it and
  wait; don't build a parallel byte-level escape hatch.
- **A `postDecodeAll` / `preEncodeAll` hook** may be added for
  stack-wide passes (e.g. computing a length that depends on the
  fully-populated upper layer). If your current design wants to reach
  `ctx.upper()->upper()`, raise a feature request rather than
  chaining adjacency.
- **`nextLayer` error reporting** — returning an unknown service name
  is discovered at dispatch time today. This will likely grow a way
  to flag "unknown but expected" vs "broken." Keep your `""`-means-
  use-static-default convention; it will map cleanly.

What will **not** change:

- `ILogicModule`'s three methods and their signatures.
- `REGISTER_LOGIC(name, Class)` macro form.
- The binding rule table above.
- Module lifetime = stack lifetime.
