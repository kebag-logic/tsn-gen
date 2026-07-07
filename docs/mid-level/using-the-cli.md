# Using the packet_gen CLI

*For test and validation engineers generating stimulus and decoding captures with `packet_gen`.*

`packet_gen` reads a directory of protocol YAML files, builds an in-memory
database of variables and interfaces, then generates or decodes packets whose
field values are constrained by those definitions. The build produces it at
`build/traffic-gen/packet_gen`; an installed package puts it on `PATH`.

The authoritative flag reference is the man page:
`man ./docs/man/packet_gen.1`. This page walks the workflows.

## The output contract

Two rules hold in every mode:

- **stdout carries only packet data** — NDJSON (default) or bare hex,
  one packet per line. Nothing else is ever printed there.
- **stderr carries all diagnostics**, filtered by `--log-level`
  (`quiet`, `error`, `warn` [default], `info`, `debug`).

So any `packet_gen` invocation is safe to pipe. Field values in JSON are
decimal integers, in YAML declaration order.

```bash
# Extract one field across five generated packets
packet_gen --yaml-dir protocols/application/1722_1/aecp \
  --interface atdecc_aecp_acquire_entity::AECP_ACQUIRE_ENTITY::AECP_ACQUIRE_ENTITY_IF \
  --count 5 | jq '.fields.sequence_id'

# Same stream into Python
packet_gen --yaml-dir protocols/application/1722_1/aecp \
  --interface atdecc_aecp_acquire_entity::AECP_ACQUIRE_ENTITY::AECP_ACQUIRE_ENTITY_IF \
  --count 5 | python3 -c '
import json, sys
for line in sys.stdin:
    pkt = json.loads(line)
    print(pkt["fields"]["sequence_id"], pkt["hex"])
'
```

Exit codes: `0` success, `1` argument error, `2` runtime error (YAML parse
failure, unknown interface, transport failure).

## Discovering interfaces

Interface names are fully qualified as `<service>::<ENTITY>::<INTERFACE>`
(see [writing-protocols.md](writing-protocols.md)). Never guess them:

```bash
packet_gen --yaml-dir protocols/ --list-interfaces
```

Point `--yaml-dir` at a subdirectory (e.g.
`protocols/application/1722_1/aecp`) to load only one protocol family, or at
`protocols/` to load everything — required when a `--stack` spans layers.

## Validating a directory

`--validate` schema-checks every YAML under `--yaml-dir` (and a
`--stack-file` if given) and exits without generating anything — a fast
authoring/CI gate:

```bash
packet_gen --yaml-dir protocols/ --validate            # exit 0 = all conform
packet_gen --yaml-dir protocols/ \
  --stack-file stacks/aecp_acquire_entity.yaml --validate
```

Diagnostics are `file:line:col: error: message` on stderr; exit code 2 means
at least one file is non-conforming. See
[writing-protocols.md](writing-protocols.md) for the schema.

## Generating packets

```bash
# One PDU, JSON on stdout
packet_gen --yaml-dir protocols/application/1722_1/aecp \
  --interface atdecc_aecp_acquire_entity::AECP_ACQUIRE_ENTITY::AECP_ACQUIRE_ENTITY_IF

# 1000 PDUs as bare hex lines
packet_gen --yaml-dir protocols/application/1722_1/aecp \
  --interface atdecc_aecp_acquire_entity::AECP_ACQUIRE_ENTITY::AECP_ACQUIRE_ENTITY_IF \
  --count 1000 --output hex

# Unlimited stream until Ctrl-C
packet_gen --yaml-dir protocols/application/1722_1/aecp \
  --interface atdecc_aecp_no_payload::AECP_NO_PAYLOAD::AECP_NO_PAYLOAD_IF \
  --count 0
```

Each field is randomised within its YAML constraints
(`expected.value`/`values` > `range` > `mask` > unconstrained).

### Seeds and reproducibility

The PRNG is a 64-bit Mersenne Twister, seeded from `/dev/urandom` unless you
pass `--seed`. Identical seed, YAML, and flags reproduce the identical packet
sequence — which makes fuzz findings replayable and lets regression tests
assert exact bytes:

```bash
packet_gen --yaml-dir protocols/application/1722_1/aecp \
  --interface atdecc_aecp_acquire_entity::AECP_ACQUIRE_ENTITY::AECP_ACQUIRE_ENTITY_IF \
  --count 100 --seed 42 --output hex > run_a.txt
# ... same command again ...
diff run_a.txt run_b.txt   # empty
```

## Chained exchanges: --connect

`--connect A:B[:C...]` generates one packet per stage per round. Fields that
stage N+1 declares *without* a hard constraint are overridden with stage N's
value when the short field names match — modelling a responder that echoes
`sequence_id`, `target_entity_id`, `controller_entity_id`, and so on:

```bash
packet_gen --yaml-dir protocols/application/1722_1/aecp \
  --connect \
atdecc_aecp_acquire_entity::AECP_ACQUIRE_ENTITY::AECP_ACQUIRE_ENTITY_IF\
:atdecc_aecp_acquire_entity::AECP_ACQUIRE_ENTITY::AECP_ACQUIRE_ENTITY_IF \
  --count 5
```

JSON output is one array per round, one object per stage, each with an
`"iface"` key. Multiple `--connect` flags append to the same chain; a
`--interface` given alongside becomes stage 0. `--connect` and `--stack` are
mutually exclusive.

## Stacked frames: --stack

`--stack A:B[:C...]` generates each layer independently (no field
propagation) and concatenates the bytes bottom-up into one on-wire frame.
Load the whole corpus so every layer's YAML resolves:

```bash
# Ethernet (14 B) + AVTP control (2 B) + AECP ACQUIRE_ENTITY (39 B) = 55 B
packet_gen --yaml-dir protocols/ \
  --stack \
ethernet_mac_frame::ETHERNET_FRAME::ETHERNET_FRAME_IF\
:avtp_control_header::AVTP_CONTROL::AVTP_CONTROL_IF\
:atdecc_aecp_acquire_entity::AECP_ACQUIRE_ENTITY::AECP_ACQUIRE_ENTITY_IF \
  --count 3 --seed 42
```

Stack JSON has a top-level `"hex"` (all bytes) plus a `"layers"` array with
per-layer `iface`, `fields`, and `hex`:

```json
{"hex":"...55 bytes...","layers":[{"iface":"ethernet_mac_frame::...","fields":{...},"hex":"..."},...]}
```

Derived cross-layer fields (e.g. a length that depends on the payload) are
*not* computed by `--stack` — it is a plain byte concatenation. To have
each layer's logic module fill and validate derived fields, use
`--stack-file` instead.

## Logic-driven stacks (`--stack-file`)

`--stack-file <yaml>` assembles a frame through a stack definition and the
per-layer logic modules declared by each service's `logic:` key. Unlike
`--stack`, the modules run: on encode they fill derived fields (EtherType,
AVTP subtype, AECP `control_data_length`, command status); on decode they
validate and drive demux. The result is a semantically valid frame rather
than random bytes in the right shape.

```bash
# Assemble a valid Ethernet + AVTP + AECP ACQUIRE_ENTITY frame
packet_gen --yaml-dir protocols/ \
  --stack-file stacks/aecp_acquire_entity.yaml --seed 42 --count 1
# → subtype=0xfb, control_data_length=36, command status forced to 0

# Decode one back through the same stack
packet_gen --yaml-dir protocols/ --stack-file stacks/aecp_acquire_entity.yaml \
  --decode --hex <hexstring>
```

A stack YAML lists services bottom-up; see
[writing-protocols.md](writing-protocols.md) for its schema and
[../low-level/logic-modules.md](../low-level/logic-modules.md) for how
modules bind. `--stack-file`, `--stack`, and `--connect` are mutually
exclusive.

## Transports

`--transport <kind>:<param>` delivers every generated packet to a backend
*in addition to* printing it on stdout. Default is `none`.

| Spec | Backend | Notes |
|------|---------|-------|
| `raw:eth0` | Linux `AF_PACKET`/`SOCK_RAW` injection | Needs `CAP_NET_RAW`: run as root or `setcap cap_net_raw+ep packet_gen` |
| `pcap:/tmp/cap.pcap` | libpcap-format file (`LINKTYPE_ETHERNET`, no libpcap dependency) | Created or truncated on open; opens in Wireshark/tcpdump |
| `verilator:/tmp/axis.sock` | AXI-Stream beats over a UNIX socket to a Verilator DUT | The simulation must create the socket first |

```bash
# Capture 100 stacked frames for Wireshark
packet_gen --yaml-dir protocols/ \
  --stack ethernet_mac_frame::ETHERNET_FRAME::ETHERNET_FRAME_IF\
:avtp_control_header::AVTP_CONTROL::AVTP_CONTROL_IF\
:atdecc_aecp_acquire_entity::AECP_ACQUIRE_ENTITY::AECP_ACQUIRE_ENTITY_IF \
  --count 100 --transport pcap:/tmp/capture.pcap

# Feed a Verilator DUT
packet_gen --yaml-dir protocols/application/1722_1/aecp \
  --interface atdecc_aecp_acquire_entity::AECP_ACQUIRE_ENTITY::AECP_ACQUIRE_ENTITY_IF \
  --transport verilator:/tmp/verilator_axis.sock
```

The verilator transport is fire-and-forget; for cycle-accurate, backpressured
co-simulation use the SystemC integration described in
[../../sim/README.md](../../sim/README.md).

## Decoding

`--decode` reverses the pipeline: bytes in, named fields out, printed in the
same NDJSON/hex formats. Sources:

```bash
# One-shot: decode a hex string from the command line (--hex implies --decode)
packet_gen --yaml-dir protocols/application/1722_1/aecp \
  --interface atdecc_aecp_acquire_entity::AECP_ACQUIRE_ENTITY::AECP_ACQUIRE_ENTITY_IF \
  --hex 1402002c000000000000000000000000

# Every packet in a pcap file
packet_gen --yaml-dir protocols/application/1722_1/aecp \
  --interface atdecc_aecp_acquire_entity::AECP_ACQUIRE_ENTITY::AECP_ACQUIRE_ENTITY_IF \
  --decode --transport pcap:/tmp/capture.pcap

# Live capture, filtered with jq
packet_gen --yaml-dir protocols/application/1722_1/aecp \
  --interface atdecc_aecp_acquire_entity::AECP_ACQUIRE_ENTITY::AECP_ACQUIRE_ENTITY_IF \
  --decode --transport raw:eth0 | jq 'select(.fields.status != 0)'
```

Decoding a multi-layer capture uses the same `--stack` list as generation;
the frame is sliced at each layer's byte offset:

```bash
packet_gen --yaml-dir protocols/ \
  --stack ethernet_mac_frame::ETHERNET_FRAME::ETHERNET_FRAME_IF\
:avtp_control_header::AVTP_CONTROL::AVTP_CONTROL_IF\
:atdecc_aecp_acquire_entity::AECP_ACQUIRE_ENTITY::AECP_ACQUIRE_ENTITY_IF \
  --decode --transport pcap:/tmp/capture.pcap
```

Round-trip sanity check — generate, then decode your own output:

```bash
packet_gen --yaml-dir protocols/application/1722_1/aecp \
  --interface atdecc_aecp_acquire_entity::AECP_ACQUIRE_ENTITY::AECP_ACQUIRE_ENTITY_IF \
  --output hex --seed 99 | \
xargs -I{} packet_gen --yaml-dir protocols/application/1722_1/aecp \
  --interface atdecc_aecp_acquire_entity::AECP_ACQUIRE_ENTITY::AECP_ACQUIRE_ENTITY_IF \
  --hex {}
```

## Miscellaneous flags

```bash
packet_gen --version          # prints "packet_gen <version>" and exits 0
packet_gen --help             # usage summary on stderr, exits 0
packet_gen ... --log-level debug   # per-file/per-field tracing on stderr
```

## See also

- YAML schema and constraints: [writing-protocols.md](writing-protocols.md)
- Driving the same pipelines from C++:
  [embedding-the-library.md](embedding-the-library.md)
- Manual page: [../man/packet_gen.1](../man/packet_gen.1)
- Documentation map: [../INDEX.md](../INDEX.md)
