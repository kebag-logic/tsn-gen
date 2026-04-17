/*
 * SPDX-FileCopyrightText: 2025 Kebag-Logic (https://kebag-logic.com)
 * SPDX-FileCopyrightText: 2025 Alexandre Malki <alexandre.malki@kebag-logic.com>
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <receiver.h>
#include <verilator_sender.h>   /* reuse AxiStreamBeat definition */
#include <string>

/**
 * @brief Receives AXI-Stream packets from a Verilator DUT over a UNIX socket.
 *
 * Reads AxiStreamBeat structs (tdata/tkeep/tlast) from the socket and
 * reassembles them into a complete packet byte buffer.  Each call to
 * receive() reads beats until tlast=1 and returns the reconstructed frame.
 *
 * The DUT is expected to have already connected as a client to @p socketPath
 * (or the socket path is one that the DUT creates as a server, in which case
 * the caller connects via open()).  This implementation acts as a client —
 * it connects to an already-listening socket.
 */
class VerilatorReceiver : public IReceiver {
public:
    explicit VerilatorReceiver(const std::string& socketPath);
    ~VerilatorReceiver() override;

    ReceiverErr open() override;
    ReceiverErr receive(std::vector<uint8_t>& packet) override;
    void close() noexcept override;

private:
    const std::string mSocketPath;
    int mSockFd;

    ReceiverErr readAll(void* buf, size_t len);
};
