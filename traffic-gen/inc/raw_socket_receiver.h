/*
 * SPDX-FileCopyrightText: 2025 Kebag-Logic (https://kebag-logic.com)
 * SPDX-FileCopyrightText: 2025 Alexandre Malki <alexandre.malki@kebag-logic.com>
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <receiver.h>
#include <string>

/**
 * @brief Captures raw Ethernet frames from a network interface using an
 *        AF_PACKET / SOCK_RAW socket.
 *
 * Requires CAP_NET_RAW (root or the appropriate capability).
 * Each receive() call returns one complete Ethernet frame as captured.
 *
 * receive() blocks until a frame arrives; use a timeout or signal to
 * interrupt from a test harness.
 */
class RawSocketReceiver : public IReceiver {
public:
    explicit RawSocketReceiver(const std::string& ifName);
    ~RawSocketReceiver() override;

    ReceiverErr open() override;
    ReceiverErr receive(std::vector<uint8_t>& packet) override;
    void close() noexcept override;

    static constexpr size_t MAX_FRAME_BYTES = 65535;

private:
    const std::string mIfName;
    int mSockFd;
};
