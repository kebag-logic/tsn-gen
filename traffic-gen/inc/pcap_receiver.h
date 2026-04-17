/*
 * SPDX-FileCopyrightText: 2025 Kebag-Logic (https://kebag-logic.com)
 * SPDX-FileCopyrightText: 2025 Alexandre Malki <alexandre.malki@kebag-logic.com>
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <receiver.h>
#include <string>

/**
 * @brief Replays packets from a libpcap-format capture file.
 *
 * Reads the global header on open(), then returns one packet per receive()
 * call in file order.  Returns RECEIVER_ERR_EOF when all packets are consumed.
 *
 * Compatible with any pcap file written by PcapSender or Wireshark.
 */
class PcapReceiver : public IReceiver {
public:
    explicit PcapReceiver(const std::string& filePath);
    ~PcapReceiver() override;

    ReceiverErr open() override;
    ReceiverErr receive(std::vector<uint8_t>& packet) override;
    void close() noexcept override;

private:
    const std::string mFilePath;
    int  mFd;
    bool mSwap;  ///< true if byte-order swap needed (big-endian pcap)
};
