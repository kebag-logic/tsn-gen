# ns-3 + Verilator Co-simulation

This directory implements a unified simulation environment where **ns-3**
(discrete-event network simulation) and **Verilator** (cycle-accurate RTL
simulation) share a single virtual clock provided by **SystemC**.

## Architecture

```
sc_main()                      ← SystemC owns the unified clock
    │
    ├── sc_clock clk (8 ns)    ← 125 MHz default; tune to your DUT
    │
    ├── VTsnEndpoint dut        ← Verilator RTL, compiled with --sc
    │     sc_in  s_axis_*       (AXI-Stream slave — packet into DUT)
    │     sc_out m_axis_*       (AXI-Stream master — packet out of DUT)
    │
    ├── AxisBridge bridge        ← sim/src/axis_bridge.cpp
    │     tx_thread: ns-3 packet → AXI beats → DUT master ports
    │     rx_thread: DUT slave ports → AXI beats → ns-3 receive event
    │
    └── ns-3 topology
          Node[0]
            └── VerilatorNetDevice ← sim/src/verilator_net_device.cpp
                  Send()  → bridge.push_tx_packet()
                  on_rx   → Simulator::ScheduleNow(DeliverPacket)
```

No sockets, no barrier synchronisation, no OS scheduling latency.  Each
ns-3 event fires at the correct SystemC virtual time, and each AXI-Stream
beat is presented to the DUT on the exact rising clock edge where the ns-3
event occurred.

## Key components

| File | Purpose |
|---|---|
| `inc/axis_bridge.h` | SC_MODULE with AXI-Stream ports and packet FIFOs |
| `src/axis_bridge.cpp` | TX/RX SC_THREADs — beat framing and reassembly |
| `inc/verilator_net_device.h` | ns-3 NetDevice backed by AxisBridge |
| `src/verilator_net_device.cpp` | Send/Receive glue between ns-3 and SystemC |
| `sc_main.cpp` | Top-level wiring of DUT, bridge, and ns-3 topology |
| `CMakeLists.txt` | Build (gated by `ENABLE_SYSTEMC_SIM=ON`) |

The `AxiStreamBeat` struct is defined in `traffic-gen/inc/axi_stream_beat.h`
and shared with the socket-based `VerilatorSender` / `VerilatorReceiver`.

## Prerequisites

| Dependency | Version | Notes |
|---|---|---|
| SystemC | ≥ 2.3.3 | IEEE 1666-2023; set `SYSTEMC_ROOT` if not in default path |
| Verilator | ≥ 5.0 | Must compile your DUT with `--sc` |
| ns-3 | ≥ 3.40 | Must be built with `--enable-systemc` |

## Build

```bash
cmake -B build \
      -DENABLE_SYSTEMC_SIM=ON \
      -DSYSTEMC_ROOT=/path/to/systemc \
      -DNS3_DIR=/path/to/ns-3/build
cmake --build build --target tsn_cosim
```

## Prepare the Verilator DUT

```bash
verilator --sc --exe --build \
          -Mdir obj_dir       \
          --top-module TsnEndpoint \
          TsnEndpoint.sv
```

The generated `obj_dir/VTsnEndpoint.h` is included by `sc_main.cpp`.
Adjust the module name and port bindings in `sc_main.cpp` to match your DUT.

## Run

```bash
./build/tsn_cosim protocols/
```

The first argument is the path to the YAML protocol definitions directory
(same format used by `packet_gen`).  Simulation stops after 1 ms by default;
edit the `sc_start()` call in `sc_main.cpp` to change this.

## Relationship to the socket-based Verilator backend

The original `VerilatorSender` / `VerilatorReceiver` (socket mode) are
**not removed**.  They remain available for deployments where SystemC is not
required — for example, running the DUT under a stand-alone Verilator
simulation that listens on a UNIX socket.  Both backends share the same
`AxiStreamBeat` wire format.

| Mode | Entry point | Time accuracy | Requires |
|---|---|---|---|
| Socket | `packet_gen --verilator /tmp/axi.sock` | None (fire-and-forget) | UNIX socket server in DUT |
| SystemC | `tsn_cosim protocols/` | Cycle-accurate | SystemC + ns-3 w/ SystemC |
