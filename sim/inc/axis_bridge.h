/*
 * SPDX-FileCopyrightText: 2025 Kebag-Logic (https://kebag-logic.com)
 * SPDX-FileCopyrightText: 2025 Alexandre Malki <alexandre.malki@kebag-logic.com>
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <systemc>
#include <axi_stream_beat.h>

#include <cstdint>
#include <functional>
#include <vector>

/**
 * @brief SystemC module bridging ns-3 packet events to Verilator AXI-Stream ports.
 *
 * This is the central coupling point between ns-3 (discrete-event scheduler)
 * and the Verilator-generated RTL DUT (cycle-accurate SystemC module).
 * The SystemC kernel owns both time domains — no socket, no serialisation,
 * no OS scheduling latency between the two simulators.
 *
 * ## TX path (ns-3 → DUT)
 *   ns-3 calls push_tx_packet() which writes into tx_fifo.
 *   tx_thread pops a packet each time one is available, frames it into
 *   AxiStreamBeat-sized chunks and drives the AXI-Stream master ports
 *   one beat per rising clock edge, respecting TREADY backpressure.
 *
 * ## RX path (DUT → ns-3)
 *   rx_thread samples the AXI-Stream slave ports each rising clock edge,
 *   reassembles beats into a complete packet and invokes on_rx (a callback
 *   registered by VerilatorNetDevice) so that ns-3 can inject the frame
 *   as a receive event via Simulator::ScheduleNow().
 *
 * ## Requirements
 *   - ns-3 must be compiled with SystemC support and use the SystemC-backed
 *     SimulatorImpl so that both event loops share a single virtual clock.
 *   - The Verilator DUT is compiled with --sc to produce a SystemC module
 *     whose AXI-Stream ports are connected directly to the signals declared
 *     in sc_main.
 */
SC_MODULE(AxisBridge) {
    /* ------------------------------------------------------------------ */
    /* Clock                                                               */
    /* ------------------------------------------------------------------ */
    sc_core::sc_in<bool> clk{"clk"};

    /* ------------------------------------------------------------------ */
    /* AXI-Stream master ports → Verilator DUT slave (TX into DUT)        */
    /* ------------------------------------------------------------------ */
    sc_core::sc_out<sc_dt::sc_uint<64>> m_tdata{"m_tdata"};
    sc_core::sc_out<sc_dt::sc_uint<8>>  m_tkeep{"m_tkeep"};
    sc_core::sc_out<bool>               m_tvalid{"m_tvalid"};
    sc_core::sc_out<bool>               m_tlast{"m_tlast"};
    sc_core::sc_in<bool>                m_tready{"m_tready"};

    /* ------------------------------------------------------------------ */
    /* AXI-Stream slave ports ← Verilator DUT master (RX from DUT)       */
    /* ------------------------------------------------------------------ */
    sc_core::sc_in<sc_dt::sc_uint<64>>  s_tdata{"s_tdata"};
    sc_core::sc_in<sc_dt::sc_uint<8>>   s_tkeep{"s_tkeep"};
    sc_core::sc_in<bool>                s_tvalid{"s_tvalid"};
    sc_core::sc_in<bool>                s_tlast{"s_tlast"};
    sc_core::sc_out<bool>               s_tready{"s_tready"};

    /* ------------------------------------------------------------------ */
    /* Interface shared with ns-3 / VerilatorNetDevice                    */
    /* ------------------------------------------------------------------ */

    /**
     * @brief Push a packet into the TX queue from the ns-3 context.
     * @return false if the FIFO is full (packet dropped).
     */
    bool push_tx_packet(std::vector<uint8_t> pkt) {
        return tx_fifo.nb_write(std::move(pkt));
    }

    /**
     * @brief Callback invoked from rx_thread when a complete packet arrives.
     *
     * Register before sc_start().  Called from the SystemC thread context —
     * the handler must use Simulator::ScheduleNow() to re-enter ns-3 safely.
     */
    std::function<void(std::vector<uint8_t>)> on_rx;

    SC_CTOR(AxisBridge) {
        SC_THREAD(tx_thread);
        sensitive << clk.pos();

        SC_THREAD(rx_thread);
        sensitive << clk.pos();
    }

private:
    /* Depth-4 packet FIFOs; one slot per packet, not per beat. */
    sc_core::sc_fifo<std::vector<uint8_t>> tx_fifo{4};

    void tx_thread();
    void rx_thread();
};
