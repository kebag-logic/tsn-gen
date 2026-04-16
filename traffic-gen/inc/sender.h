/*
 * SPDX-FileCopyrightText: 2025 Kebag-Logic (https://kebag-logic.com)
 * SPDX-FileCopyrightText: 2025 Alexandre Malki <alexandre.malki@kebag-logic.com>
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <cstdint>
#include <vector>
#include <string>

#include <error.h>

class SenderErr : public Error {
public:
    enum {
        SENDER_SUCCESS,
        SENDER_ERR_UNKNOWN,
        SENDER_ERR_OPEN_FAILED,
        SENDER_ERR_SEND_FAILED,
        SENDER_ERR_NOT_OPEN,

        SENDER_ERR_ENUM_SENTINEL
    };

    SenderErr() = delete;
    explicit SenderErr(int code) : Error(code, mErrVector) {}

    SenderErr& operator=(const SenderErr other)
    {
        mErrorCode = other.mErrorCode;
        return *this;
    }

private:
    const std::vector<std::string> mErrVector = {
        "SENDER: SUCCESS",
        "SENDER: ERR: UNKNOWN",
        "SENDER: ERR: OPEN FAILED",
        "SENDER: ERR: SEND FAILED",
        "SENDER: ERR: NOT OPEN",
    };
};

/**
 * @brief Abstract transport layer for the traffic generator.
 *
 * Concrete implementations:
 *   - RawSocketSender  — Linux AF_PACKET raw socket (real network interface)
 *   - VerilatorSender  — AXI-Stream beats over a UNIX socket (Verilator DUT)
 *
 * Lifecycle: open() → send() × N → close()
 */
class ISender {
public:
    virtual ~ISender() = default;

    /**
     * @brief Acquire the underlying resource (socket fd, connection, …).
     * @return SENDER_SUCCESS or SENDER_ERR_OPEN_FAILED.
     */
    virtual const SenderErr open() = 0;

    /**
     * @brief Transmit one packet.
     * @param packet  Raw byte buffer to send (host byte order, no framing added).
     * @return SENDER_SUCCESS or SENDER_ERR_SEND_FAILED / SENDER_ERR_NOT_OPEN.
     */
    virtual const SenderErr send(const std::vector<uint8_t>& packet) = 0;

    /**
     * @brief Release the underlying resource.  Best-effort; must not throw.
     */
    virtual void close() noexcept = 0;
};
