/*
 * SPDX-FileCopyrightText: 2025 Kebag-Logic (https://kebag-logic.com)
 * SPDX-FileCopyrightText: 2025 Alexandre Malki <alexandre.malki@kebag-logic.com>
 * SPDX-License-Identifier: MIT
 */

#include <verilator_sender.h>

#include <cerrno>
#include <cstring>
#include <iostream>

#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

VerilatorSender::VerilatorSender(const std::string& socketPath)
    : mSocketPath(socketPath), mSockFd(-1)
{
}

VerilatorSender::~VerilatorSender()
{
    close();
}

const SenderErr VerilatorSender::open()
{
    mSockFd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (mSockFd < 0) {
        std::cerr << "VerilatorSender: socket() failed: "
                  << std::strerror(errno) << "\n";
        return SenderErr(SenderErr::SENDER_ERR_OPEN_FAILED);
    }

    struct sockaddr_un addr = {};
    addr.sun_family = AF_UNIX;
    std::strncpy(addr.sun_path, mSocketPath.c_str(),
                 sizeof(addr.sun_path) - 1);

    if (connect(mSockFd,
                reinterpret_cast<struct sockaddr*>(&addr),
                sizeof(addr)) < 0) {
        std::cerr << "VerilatorSender: connect() to '" << mSocketPath
                  << "' failed: " << std::strerror(errno) << "\n";
        ::close(mSockFd);
        mSockFd = -1;
        return SenderErr(SenderErr::SENDER_ERR_OPEN_FAILED);
    }

    return SenderErr(SenderErr::SENDER_SUCCESS);
}

const SenderErr VerilatorSender::writeAll(const void* data, size_t len)
{
    const uint8_t* ptr = reinterpret_cast<const uint8_t*>(data);
    size_t remaining = len;

    while (remaining > 0) {
        const ssize_t n = write(mSockFd, ptr, remaining);
        if (n < 0) {
            if (errno == EINTR) continue;
            std::cerr << "VerilatorSender: write() failed: "
                      << std::strerror(errno) << "\n";
            return SenderErr(SenderErr::SENDER_ERR_SEND_FAILED);
        }
        ptr       += n;
        remaining -= static_cast<size_t>(n);
    }
    return SenderErr(SenderErr::SENDER_SUCCESS);
}

const SenderErr VerilatorSender::send(const std::vector<uint8_t>& packet)
{
    if (mSockFd < 0) {
        return SenderErr(SenderErr::SENDER_ERR_NOT_OPEN);
    }

    if (packet.empty()) {
        return SenderErr(SenderErr::SENDER_SUCCESS);
    }

    const size_t totalBytes = packet.size();
    size_t offset = 0;

    while (offset < totalBytes) {
        const size_t remaining = totalBytes - offset;
        const size_t chunkSize =
            (remaining >= TDATA_BYTES) ? TDATA_BYTES : remaining;
        const bool isLast = (offset + chunkSize >= totalBytes);

        AxiStreamBeat beat = {};

        /* Pack bytes into tdata little-endian: byte[0] → bits[7:0]. */
        for (size_t i = 0; i < chunkSize; ++i) {
            beat.tdata |= static_cast<uint64_t>(packet[offset + i])
                          << (i * 8);
        }

        /* tkeep: bit i is set if byte i is valid. */
        beat.tkeep = static_cast<uint8_t>((1u << chunkSize) - 1u);
        beat.tlast = isLast ? 1u : 0u;

        const SenderErr err = writeAll(&beat, sizeof(beat));
        if (err.getErrorCode() != SenderErr::SENDER_SUCCESS) {
            return err;
        }

        offset += chunkSize;
    }

    return SenderErr(SenderErr::SENDER_SUCCESS);
}

void VerilatorSender::close() noexcept
{
    if (mSockFd >= 0) {
        ::close(mSockFd);
        mSockFd = -1;
    }
}
