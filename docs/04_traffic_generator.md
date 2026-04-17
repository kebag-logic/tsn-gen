# Traffic Generator

The `traffic_gen` shared library (`libtraffic_gen.so`) builds raw packet
payloads from parsed protocol definitions and delivers them through a
pluggable transport layer.

---

## Component overview

```
TrafficGenerator
    ├── PacketBuilder       Turns interface var-refs into a byte buffer
    └── ISender             Abstract transport (strategy pattern)
          ├── RawSocketSender    Linux AF_PACKET — injects onto a real NIC
          └── VerilatorSender    AXI-Stream over UNIX socket → Verilator DUT
```

---

## TrafficGenerator

Header: `traffic-gen/inc/traffic_generator.h`

```cpp
TrafficGenerator(const ProtocolParser& parser,
                 std::unique_ptr<ISender> sender);

const TrafficGeneratorErr open();
void                      close() noexcept;

const TrafficGeneratorErr send(const std::string& qualifiedName);
const TrafficGeneratorErr sendFiltered(ProtocolInterface::IfDirections dir);
const TrafficGeneratorErr sendAll();
const TrafficGeneratorErr sendLoop(const std::string& qualifiedName,
                                   size_t count);
void seedRng(uint64_t s);
```

### Lifecycle

```
construct → open() → send*(…) → close()
```

`send*` methods return `TGEN_ERR_NOT_OPEN` if called before `open()`.

### Error codes

| Code | Meaning |
|------|---------|
| `TGEN_SUCCESS` | Operation succeeded |
| `TGEN_ERR_NOT_OPEN` | `open()` has not been called |
| `TGEN_ERR_IFACE_NOT_FOUND` | Named interface not in the database |
| `TGEN_ERR_SEND_FAILED` | The underlying sender returned an error |
| `TGEN_ERR_EMPTY_PACKET` | Interface has no resolvable var refs — packet skipped (non-fatal in `sendAll`/`sendFiltered`) |

### Methods

#### `send(qualifiedName)`

Builds and sends one packet for the named interface.

```cpp
gen.send("simple_service::SIMPLE_SERVICE_ENTITY::SIMPLE_INTERFACE");
```

#### `sendFiltered(dir)`

Iterates every interface in the database and sends a packet for each one
whose direction matches `dir`. Empty packets are silently skipped (no error
propagated to the caller).

```cpp
gen.sendFiltered(ProtocolInterface::IN);   // feed all DUT inputs
```

#### `sendAll()`

Equivalent to `sendFiltered` without a direction constraint. Sends a packet
for every interface, regardless of direction.

#### `sendLoop(qualifiedName, count)`

Sends `count` packets for the named interface in sequence. Pass `count = 0`
for an infinite loop (useful for continuous traffic generation).

```cpp
gen.sendLoop("simple_service::SIMPLE_SERVICE_ENTITY::SIMPLE_INTERFACE", 1000);
```

#### `seedRng(s)`

Seeds the `PacketBuilder`'s internal `mt19937_64`. Call before the first
`send*` to get reproducible packet sequences.

```cpp
gen.seedRng(42);
```

---

## PacketBuilder

Header: `traffic-gen/inc/packet_builder.h`  
Source: `traffic-gen/src/packet_builder.cpp`

Translates a `ProtocolInterface` into a raw byte buffer.

### Bit-packing rules

Fields are packed **MSB-first** into consecutive bit positions. The resulting
buffer is padded to the next byte boundary.

Example: two fields, `field_a` (4 bits, value `0b1010`) and `field_b`
(3 bits, value `0b101`):

```
Bit position:  7  6  5  4  3  2  1  0
               ──────────────────────
Byte 0:        1  0  1  0  1  0  1  0
               └─────────┘ └──────┘ └─ padding
               field_a=0xA  field_b=5
```

### Value selection

For each var:
1. If `getExpectedValues()` is non-empty, pick one uniformly at random.
2. Otherwise generate a random integer in `[0, 2^size − 1]`.

Var refs that resolve to `nullptr` in the database are skipped with a warning.

### API

```cpp
PacketBuilder builder;
builder.seed(42);

std::vector<uint8_t> pkt =
    builder.build(iface, parser.getVarDatabase());
// empty if the interface has no resolvable var refs
```

---

## ISender

Header: `traffic-gen/inc/sender.h`

Abstract interface for the transport layer.

```cpp
class ISender {
public:
    virtual const SenderErr open() = 0;
    virtual const SenderErr send(const std::vector<uint8_t>& packet) = 0;
    virtual void close() noexcept = 0;
};
```

Lifecycle: `open()` → `send()` × N → `close()`.

### Error codes

| Code | Meaning |
|------|---------|
| `SENDER_SUCCESS` | OK |
| `SENDER_ERR_OPEN_FAILED` | Could not acquire the resource |
| `SENDER_ERR_SEND_FAILED` | Transmission error |
| `SENDER_ERR_NOT_OPEN` | `send()` called before `open()` |

---

## RawSocketSender

Header: `traffic-gen/inc/raw_socket_sender.h`  
Source: `traffic-gen/src/raw_socket_sender.cpp`

Injects raw Ethernet frames onto a Linux network interface.

```cpp
RawSocketSender sender("eth0");
sender.open();
sender.send(packet);
sender.close();
```

**Requires `CAP_NET_RAW`** (run as root or grant the capability with
`setcap cap_net_raw+eip`).

### Implementation notes

- Creates an `AF_PACKET / SOCK_RAW` socket with `ETH_P_ALL`.
- Resolves the interface index via `ioctl(SIOCGIFINDEX)`.
- Binds to the interface using `sockaddr_ll`.
- Sends with `sendto()` and a `sockaddr_ll` addressing structure.
- `close()` is idempotent and noexcept.

---

## VerilatorSender

Header: `traffic-gen/inc/verilator_sender.h`  
Source: `traffic-gen/src/verilator_sender.cpp`

Sends packets to a Verilator DUT via a UNIX stream socket using the
AXI-Stream protocol.

```cpp
VerilatorSender sender("/tmp/verilator_axi.sock");
sender.open();    // connects; the Verilator wrapper must be listening
sender.send(packet);
sender.close();
```

### AXI-Stream framing

Each packet is fragmented into a sequence of `AxiStreamBeat` structs:

```cpp
struct AxiStreamBeat {
    uint64_t tdata;  // 8 payload bytes, first byte in LSB (little-endian)
    uint8_t  tkeep;  // byte-enable bitmask
    uint8_t  tlast;  // 1 on the last beat of the packet
} __attribute__((packed));  // sizeof == 10
```

| Field | Full beat | Last (partial) beat |
|-------|-----------|---------------------|
| `tdata` | 8 payload bytes | remaining bytes in LSB positions |
| `tkeep` | `0xFF` | `(1 << remainingBytes) − 1` |
| `tlast` | `0` | `1` |

Example: 10-byte packet → 2 beats:

```
Beat 0:  tdata = bytes[0..7]  tkeep = 0xFF  tlast = 0
Beat 1:  tdata = bytes[8..9]  tkeep = 0x03  tlast = 1
```

The UNIX socket path is the only connection parameter; there is no in-band
flow-control handshake — the OS socket buffer provides backpressure.

### Verilator wrapper contract

The Verilator side must:
1. Create a UNIX `SOCK_STREAM` socket and listen on the configured path
   **before** `VerilatorSender::open()` is called.
2. Read `AxiStreamBeat` structs (10 bytes each) in a loop.
3. Drive the DUT AXI-Stream input signals (`TDATA`, `TKEEP`, `TLAST`,
   `TVALID`) from each beat.

---

## Choosing a sender

```cpp
// ---- Real network interface ----
auto sender = std::make_unique<RawSocketSender>("eth0");

// ---- Verilator co-simulation ----
auto sender = std::make_unique<VerilatorSender>("/tmp/vrl.sock");

// ---- Unit testing (capture only, no hardware needed) ----
// Implement ISender in your test code (see CaptureSender in traffic_gen_test.cpp)
```

All three are interchangeable: pass any of them to `TrafficGenerator` via
`std::unique_ptr<ISender>`.

---

## Full example

```cpp
#include <protocol_parser.h>
#include <traffic_generator.h>
#include <verilator_sender.h>

int main() {
    // 1. Parse protocols
    ProtocolParser parser("/path/to/protocols");
    if (parser.parse().getErrorCode() != ProtocolParserErr::PROTOPARSER_SUCCESS) {
        return 1;
    }

    // 2. Choose transport
    auto sender = std::make_unique<VerilatorSender>("/tmp/vrl.sock");

    // 3. Create generator
    TrafficGenerator gen(parser, std::move(sender));
    gen.seedRng(12345);

    // 4. Run
    gen.open();
    gen.sendFiltered(ProtocolInterface::IN);   // send one packet per IN interface
    gen.close();

    return 0;
}
```
