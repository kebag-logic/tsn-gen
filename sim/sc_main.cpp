/*
 * SPDX-FileCopyrightText: 2025 Kebag-Logic (https://kebag-logic.com)
 * SPDX-FileCopyrightText: 2025 Alexandre Malki <alexandre.malki@kebag-logic.com>
 * SPDX-License-Identifier: MIT
 */

/*
 * Top-level SystemC entry point for the ns-3 + Verilator co-simulation.
 *
 * SystemC owns the unified virtual clock.  Both ns-3 events and Verilator
 * RTL cycles are driven from this single sc_main(), which removes the need
 * for any barrier synchronisation between the two simulators.
 *
 * ## How to use
 *
 * 1. Compile your RTL DUT with Verilator:
 *
 *      verilator --sc --exe --build -Mdir obj_dir \
 *                --top-module TsnEndpoint          \
 *                TsnEndpoint.sv
 *
 *    This generates obj_dir/VTsnEndpoint.h (and VTsnEndpoint__Syms.h etc.).
 *    Replace VTsnEndpoint below with the class name matching your top module.
 *
 * 2. Configure the build with SystemC and ns-3 found:
 *
 *      cmake -B build -DENABLE_SYSTEMC_SIM=ON \
 *            -DSYSTEMC_ROOT=/path/to/systemc   \
 *            -DNS3_ROOT=/path/to/ns-3
 *
 * 3. Adjust the clock period (currently 8 ns = 125 MHz) and the AXI-Stream
 *    port names to match your DUT's port list.
 *
 * 4. Set the simulation stop time via the NS3_SIM_DURATION_NS environment
 *    variable or by editing the sc_start() call below.
 */

#include <systemc>

/* Verilator-generated SystemC module — produced by `verilator --sc`.
 * Replace with the actual header generated for your DUT. */
#include "VTsnEndpoint.h"

#include <axis_bridge.h>
#include <verilator_net_device.h>

/* ns-3 headers */
#include <ns3/core-module.h>
#include <ns3/network-module.h>
#include <ns3/internet-module.h>
#include <ns3/applications-module.h>

/* tsn-gen headers */
#include <protocol_parser.h>
#include <traffic_generator.h>

/* -----------------------------------------------------------------------
 * Helper: build a minimal ns-3 topology with one VerilatorNetDevice node.
 * Returns the node so TsnGenApp can be attached to it.
 * ----------------------------------------------------------------------- */
static ns3::Ptr<ns3::Node> BuildNs3Topology(VerilatorNetDevice* dev)
{
    ns3::NodeContainer nodes;
    nodes.Create(1);

    ns3::Ptr<ns3::Node> node = nodes.Get(0);

    /* Attach our Verilator-backed device. */
    dev->SetAddress(ns3::Mac48Address::Allocate());
    node->AddDevice(ns3::Ptr<ns3::NetDevice>(dev));

    return node;
}

/* -----------------------------------------------------------------------
 * sc_main
 * ----------------------------------------------------------------------- */
int sc_main(int argc, char* argv[])
{
    /* ------------------------------------------------------------------ */
    /* 1. Clock — 125 MHz matches a typical TSN endpoint clock.           */
    /* ------------------------------------------------------------------ */
    sc_core::sc_clock clk("clk", sc_core::sc_time(8, sc_core::SC_NS));

    /* ------------------------------------------------------------------ */
    /* 2. AXI-Stream signals connecting bridge ↔ DUT.                    */
    /*    Signal names must match the DUT's port list.                    */
    /* ------------------------------------------------------------------ */
    sc_core::sc_signal<sc_dt::sc_uint<64>> sig_m_tdata{"m_tdata"};
    sc_core::sc_signal<sc_dt::sc_uint<8>>  sig_m_tkeep{"m_tkeep"};
    sc_core::sc_signal<bool>               sig_m_tvalid{"m_tvalid"};
    sc_core::sc_signal<bool>               sig_m_tlast{"m_tlast"};
    sc_core::sc_signal<bool>               sig_m_tready{"m_tready"};

    sc_core::sc_signal<sc_dt::sc_uint<64>> sig_s_tdata{"s_tdata"};
    sc_core::sc_signal<sc_dt::sc_uint<8>>  sig_s_tkeep{"s_tkeep"};
    sc_core::sc_signal<bool>               sig_s_tvalid{"s_tvalid"};
    sc_core::sc_signal<bool>               sig_s_tlast{"s_tlast"};
    sc_core::sc_signal<bool>               sig_s_tready{"s_tready"};

    /* ------------------------------------------------------------------ */
    /* 3. Verilator DUT                                                    */
    /* ------------------------------------------------------------------ */
    VTsnEndpoint dut("dut");
    dut.clk(clk);

    /* Map DUT ports — adjust to match your generated header. */
    dut.s_axis_tdata(sig_m_tdata);   /* bridge master → DUT slave  */
    dut.s_axis_tkeep(sig_m_tkeep);
    dut.s_axis_tvalid(sig_m_tvalid);
    dut.s_axis_tlast(sig_m_tlast);
    dut.s_axis_tready(sig_m_tready);

    dut.m_axis_tdata(sig_s_tdata);   /* DUT master → bridge slave  */
    dut.m_axis_tkeep(sig_s_tkeep);
    dut.m_axis_tvalid(sig_s_tvalid);
    dut.m_axis_tlast(sig_s_tlast);
    dut.m_axis_tready(sig_s_tready);

    /* ------------------------------------------------------------------ */
    /* 4. AxisBridge — connects the two time domains.                     */
    /* ------------------------------------------------------------------ */
    AxisBridge bridge("bridge");
    bridge.clk(clk);

    bridge.m_tdata(sig_m_tdata);
    bridge.m_tkeep(sig_m_tkeep);
    bridge.m_tvalid(sig_m_tvalid);
    bridge.m_tlast(sig_m_tlast);
    bridge.m_tready(sig_m_tready);

    bridge.s_tdata(sig_s_tdata);
    bridge.s_tkeep(sig_s_tkeep);
    bridge.s_tvalid(sig_s_tvalid);
    bridge.s_tlast(sig_s_tlast);
    bridge.s_tready(sig_s_tready);

    /* ------------------------------------------------------------------ */
    /* 5. ns-3 setup                                                      */
    /* ------------------------------------------------------------------ */
    ns3::GlobalValue::Bind("SimulatorImplementationType",
                           ns3::StringValue("ns3::RealtimeSimulatorImpl"));

    auto* dev = new VerilatorNetDevice(&bridge);
    dev->SetupRx();

    ns3::Ptr<ns3::Node> node = BuildNs3Topology(dev);

    /* Parse protocol definitions and attach a TrafficGenerator app. */
    const std::string protoDir = (argc > 1) ? argv[1] : "protocols/";
    ProtocolParser parser(protoDir);
    parser.parse();

    TrafficGenerator tgen(parser.getInterfaceDatabase(),
                          parser.getVarDatabase(),
                          std::unique_ptr<ISender>(dev->GetSender()));

    /* Schedule the first send at t=100 ns to allow the DUT to come out of
     * reset.  Subsequent sends are scheduled by TsnGenApp (not shown). */
    ns3::Simulator::Schedule(ns3::NanoSeconds(100),
                             [&tgen]() { tgen.sendFiltered(OUT); });

    /* ------------------------------------------------------------------ */
    /* 6. Run — SystemC kernel drives both ns-3 events and RTL cycles.   */
    /* ------------------------------------------------------------------ */
    const double duration_ns = 1e6;  /* 1 ms default; override as needed */
    sc_core::sc_start(duration_ns, sc_core::SC_NS);

    ns3::Simulator::Destroy();
    return 0;
}
