/*
 * SPDX-FileCopyrightText: 2025 Kebag-Logic (https://kebag-logic.com)
 * SPDX-FileCopyrightText: 2025 Alexandre Malki <alexandre.malki@kebag-logic.com>
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <sender.h>
#include <cstdint>
#include <string>

/**
 * @brief AXI-Stream beat (64-bit TDATA width).
 *
 * Wire layout on the UNIX socket: a stream of packed AxiStreamBeat structs.
 * The Verilator wrapper on the other end reads beats and drives the
 * corresponding DUT input signals.
 *
 * Byte order of tdata: little-endian (first packet byte in bits [7:0]).
 */
struct AxiStreamBeat {
    uint64_t tdata;  ///< 8 data bytes, first byte in LSB
    uint8_t  tkeep;  ///< byte-enable mask (bit i = byte i is valid)
    uint8_t  tlast;  ///< 1 on the last beat of the packet
} __attribute__((packed));

static_assert(sizeof(AxiStreamBeat) == 10,
              "AxiStreamBeat must be exactly 10 bytes");

/**
 * @brief Sends packets to a Verilator DUT as 64-bit AXI-Stream beats over
 *        a UNIX stream socket.
 *
 * The Verilator side connects to @p socketPath as a client and reads
 * AxiStreamBeat structs sequentially.  There is no flow-control handshake
 * at the socket level; backpressure is handled by the OS send buffer.
 *
 * Usage:
 *   VerilatorSender sender("/tmp/verilator_axi.sock");
 *   sender.open();          // connect()s to the socket; fails if no listener
 *   sender.send(packet);    // frames into beats and writes to socket
 *   sender.close();
 *
 * Packet framing:
 *   - Each beat carries 8 bytes of payload (TDATA_WIDTH = 64 bits).
 *   - tkeep = 0xFF for full beats; for the last (possibly partial) beat,
 *     tkeep has exactly the valid bytes set.
 *   - tlast = 1 on the last beat.
 */
class VerilatorSender : public ISender {
public:
    explicit VerilatorSender(const std::string& socketPath);
    ~VerilatorSender() override;

    const SenderErr open() override;
    const SenderErr send(const std::vector<uint8_t>& packet) override;
    void close() noexcept override;

    static constexpr size_t TDATA_BYTES = 8;  ///< 64-bit bus → 8 bytes/beat

private:
    const std::string mSocketPath;
    int mSockFd;

    /** Write exactly @p len bytes from @p data, retrying on EINTR. */
    const SenderErr writeAll(const void* data, size_t len);
};
