/*
 * SPDX-FileCopyrightText: 2025 Kebag-Logic (https://kebag-logic.com)
 * SPDX-FileCopyrightText: 2025 Alexandre Malki <alexandre.malki@kebag-logic.com>
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <sender.h>
#include <string>

/**
 * @brief Sends packets directly onto a network interface using a Linux
 *        AF_PACKET / SOCK_RAW socket.
 *
 * Requires CAP_NET_RAW (root or the appropriate capability).
 * No Ethernet framing is added; the packet bytes are written verbatim.
 *
 * Usage:
 *   RawSocketSender sender("eth0");
 *   sender.open();
 *   sender.send(packet);
 *   sender.close();
 */
class RawSocketSender : public ISender {
public:
    explicit RawSocketSender(const std::string& ifName);
    ~RawSocketSender() override;

    const SenderErr open() override;
    const SenderErr send(const std::vector<uint8_t>& packet) override;
    void close() noexcept override;

private:
    const std::string mIfName;
    int mSockFd;
    int mIfIndex;
};
