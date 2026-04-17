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

class ReceiverErr : public Error {
public:
    enum {
        RECEIVER_SUCCESS,
        RECEIVER_ERR_UNKNOWN,
        RECEIVER_ERR_OPEN_FAILED,
        RECEIVER_ERR_RECV_FAILED,
        RECEIVER_ERR_NOT_OPEN,
        RECEIVER_ERR_EOF,        ///< No more packets available

        RECEIVER_ERR_ENUM_SENTINEL
    };

    ReceiverErr() = delete;
    explicit ReceiverErr(int code) : Error(code, mErrVector) {}

    ReceiverErr& operator=(const ReceiverErr other)
    {
        mErrorCode = other.mErrorCode;
        return *this;
    }

private:
    const std::vector<std::string> mErrVector = {
        "RECEIVER: SUCCESS",
        "RECEIVER: ERR: UNKNOWN",
        "RECEIVER: ERR: OPEN FAILED",
        "RECEIVER: ERR: RECV FAILED",
        "RECEIVER: ERR: NOT OPEN",
        "RECEIVER: ERR: EOF",
    };
};

/**
 * @brief Symmetric counterpart to ISender — receives one raw packet at a time.
 *
 * Concrete implementations:
 *   PcapReceiver       — replay packets from a libpcap file
 *   RawSocketReceiver  — capture from a Linux AF_PACKET socket
 *   VerilatorReceiver  — read AXI-Stream beats from a UNIX socket (DUT output)
 *
 * Lifecycle: open() → receive() × N → close()
 */
class IReceiver {
public:
    virtual ~IReceiver() = default;

    /** Acquire the underlying resource. */
    virtual ReceiverErr open() = 0;

    /**
     * @brief Receive one packet into @p packet.
     * @return RECEIVER_SUCCESS, RECEIVER_ERR_EOF when no more packets, or
     *         RECEIVER_ERR_RECV_FAILED on error.
     */
    virtual ReceiverErr receive(std::vector<uint8_t>& packet) = 0;

    /** Release the underlying resource.  Best-effort; must not throw. */
    virtual void close() noexcept = 0;
};
