/*
 * SPDX-FileCopyrightText: 2025 Kebag-Logic (https://kebag-logic.com)
 * SPDX-FileCopyrightText: 2025 Alexandre Malki <alexandre.malki@kebag-logic.com>
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <sender.h>
#include <string>

/**
 * @brief Writes packets to a libpcap-format file (no external dependency).
 *
 * The file uses LINKTYPE_ETHERNET (1) with a 65535-byte snap length.
 * Each call to send() appends one packet record; timestamps are set to
 * wall-clock time at send time.
 *
 * The file can be inspected with Wireshark, tcpdump, or any pcap reader.
 *
 * Usage:
 *   PcapSender sink("/tmp/capture.pcap");
 *   sink.open();
 *   sink.send(packet);
 *   sink.close();
 */
class PcapSender : public ISender {
public:
    explicit PcapSender(const std::string& filePath);
    ~PcapSender() override;

    const SenderErr open() override;
    const SenderErr send(const std::vector<uint8_t>& packet) override;
    void close() noexcept override;

private:
    const std::string mFilePath;
    int mFd;
};
