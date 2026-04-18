# Traffic generator

Provides the traffic necessary to test an interface.
The traffic generated may be generated using the following solutions:

  * Fuzzy looping
  * Replaying capture
  * Simulation:
    * In a real environment with VMs, Dockers, Real Machines.
    * Using the NS-3 network simulator (see `sim/`)
    * Using Verilator for cycle-accurate RTL simulation (socket mode)
    * Using NS-3 + Verilator co-simulation via SystemC (see `sim/`)

## Transport backends

| Backend | Class | Description |
|---|---|---|
| Raw socket | `RawSocketSender` / `RawSocketReceiver` | Linux AF_PACKET — real NIC |
| PCAP | `PcapSender` / `PcapReceiver` | Replay / capture `.pcap` files |
| Verilator (socket) | `VerilatorSender` / `VerilatorReceiver` | AXI-Stream beats over UNIX socket |
| Verilator (SystemC) | `AxisBridge` + `VerilatorNetDevice` | Direct signal coupling — no socket (see `sim/`) |

## AXI-Stream beat format

All Verilator transports share the `AxiStreamBeat` struct (defined in
`inc/axi_stream_beat.h`):

```
 63                  0
 tdata[63:0]  — 8 payload bytes, first byte in bits [7:0]
 tkeep[7:0]   — byte-enable mask (bit i = byte i valid)
 tlast        — 1 on the last beat of the packet
```
