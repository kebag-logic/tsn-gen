/*
 * SPDX-FileCopyrightText: 2025 Kebag-Logic (https://kebag-logic.com)
 * SPDX-FileCopyrightText: 2025 Alexandre Malki <alexandre.malki@kebag-logic.com>
 * SPDX-License-Identifier: MIT
 */

#include <axis_bridge.h>

/* -------------------------------------------------------------------------
 * TX thread — drives AXI-Stream master ports toward the Verilator DUT.
 *
 * Protocol:
 *   1. Deassert TVALID and wait for a packet to arrive in the FIFO.
 *   2. For each beat: present TDATA/TKEEP/TLAST, assert TVALID.
 *   3. Advance one clock and wait until TREADY is sampled high (backpressure).
 *   4. After TLAST is accepted, deassert TVALID for one idle cycle.
 * ------------------------------------------------------------------------- */
void AxisBridge::tx_thread()
{
    m_tvalid.write(false);
    m_tlast.write(false);
    m_tdata.write(0);
    m_tkeep.write(0);
    wait();

    while (true) {
        std::vector<uint8_t> packet;
        tx_fifo.read(packet);  // blocks until push_tx_packet() delivers one

        const size_t total = packet.size();
        size_t offset = 0;

        while (offset < total) {
            const size_t remaining = total - offset;
            const size_t chunk = (remaining >= AXIS_TDATA_BYTES)
                                 ? AXIS_TDATA_BYTES : remaining;
            const bool last = (offset + chunk >= total);

            uint64_t tdata = 0;
            for (size_t i = 0; i < chunk; ++i)
                tdata |= static_cast<uint64_t>(packet[offset + i]) << (i * 8);

            m_tdata.write(static_cast<sc_dt::sc_uint<64>>(tdata));
            m_tkeep.write(static_cast<sc_dt::sc_uint<8>>((1u << chunk) - 1u));
            m_tlast.write(last);
            m_tvalid.write(true);

            /* Respect TREADY backpressure: hold signals until accepted. */
            do { wait(); } while (!m_tready.read());

            offset += chunk;
        }

        /* One idle cycle between packets. */
        m_tvalid.write(false);
        m_tlast.write(false);
        wait();
    }
}

/* -------------------------------------------------------------------------
 * RX thread — samples AXI-Stream slave ports from the Verilator DUT.
 *
 * Protocol:
 *   1. Assert TREADY (always ready to accept).
 *   2. When TVALID is seen, accumulate bytes according to TKEEP.
 *   3. When TLAST is seen, fire on_rx with the reassembled packet.
 * ------------------------------------------------------------------------- */
void AxisBridge::rx_thread()
{
    s_tready.write(true);
    wait();

    while (true) {
        /* Wait for DUT to start a packet. */
        while (!s_tvalid.read()) wait();

        std::vector<uint8_t> packet;

        while (true) {
            const uint64_t tdata = s_tdata.read().to_uint64();
            const uint8_t  tkeep = static_cast<uint8_t>(s_tkeep.read().to_uint());
            const bool     last  = s_tlast.read();

            for (int i = 0; i < static_cast<int>(AXIS_TDATA_BYTES); ++i) {
                if (tkeep & (1u << i))
                    packet.push_back(static_cast<uint8_t>(tdata >> (i * 8)));
            }

            wait();

            if (last) break;
            while (!s_tvalid.read()) wait();
        }

        if (on_rx)
            on_rx(std::move(packet));
    }
}
