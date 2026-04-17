/*
 * SPDX-FileCopyrightText: 2025 Kebag-Logic (https://kebag-logic.com)
 * SPDX-FileCopyrightText: 2025 Alexandre Malki <alexandre.malki@kebag-logic.com>
 * SPDX-License-Identifier: MIT
 */

#include <verilator_receiver.h>

#include <cerrno>
#include <cstring>
#include <iostream>

#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

VerilatorReceiver::VerilatorReceiver(const std::string& socketPath)
    : mSocketPath(socketPath), mSockFd(-1)
{
}

VerilatorReceiver::~VerilatorReceiver()
{
    close();
}

ReceiverErr VerilatorReceiver::open()
{
    mSockFd = ::socket(AF_UNIX, SOCK_STREAM, 0);
    if (mSockFd < 0) {
        std::cerr << "VerilatorReceiver: socket() failed: "
                  << std::strerror(errno) << "\n";
        return ReceiverErr(ReceiverErr::RECEIVER_ERR_OPEN_FAILED);
    }

    struct sockaddr_un addr = {};
    addr.sun_family = AF_UNIX;
    std::strncpy(addr.sun_path, mSocketPath.c_str(), sizeof(addr.sun_path) - 1);

    if (::connect(mSockFd, reinterpret_cast<struct sockaddr*>(&addr),
                  sizeof(addr)) < 0) {
        std::cerr << "VerilatorReceiver: connect('" << mSocketPath
                  << "') failed: " << std::strerror(errno) << "\n";
        ::close(mSockFd);
        mSockFd = -1;
        return ReceiverErr(ReceiverErr::RECEIVER_ERR_OPEN_FAILED);
    }

    return ReceiverErr(ReceiverErr::RECEIVER_SUCCESS);
}

ReceiverErr VerilatorReceiver::readAll(void* buf, size_t len)
{
    uint8_t* p = reinterpret_cast<uint8_t*>(buf);
    size_t rem = len;
    while (rem > 0) {
        const ssize_t n = ::read(mSockFd, p, rem);
        if (n == 0) return ReceiverErr(ReceiverErr::RECEIVER_ERR_EOF);
        if (n < 0) {
            if (errno == EINTR) continue;
            std::cerr << "VerilatorReceiver: read() failed: "
                      << std::strerror(errno) << "\n";
            return ReceiverErr(ReceiverErr::RECEIVER_ERR_RECV_FAILED);
        }
        p   += n;
        rem -= static_cast<size_t>(n);
    }
    return ReceiverErr(ReceiverErr::RECEIVER_SUCCESS);
}

ReceiverErr VerilatorReceiver::receive(std::vector<uint8_t>& packet)
{
    if (mSockFd < 0) return ReceiverErr(ReceiverErr::RECEIVER_ERR_NOT_OPEN);

    packet.clear();

    while (true) {
        AxiStreamBeat beat{};
        const auto err = readAll(&beat, sizeof(AxiStreamBeat));
        if (err.getErrorCode() != ReceiverErr::RECEIVER_SUCCESS)
            return err;

        /* Extract valid bytes according to tkeep bitmask (bit i → byte i). */
        for (int i = 0; i < static_cast<int>(VerilatorSender::TDATA_BYTES); ++i) {
            if (beat.tkeep & (1u << i)) {
                packet.push_back(static_cast<uint8_t>(beat.tdata >> (i * 8)));
            }
        }

        if (beat.tlast)
            break;
    }

    return ReceiverErr(ReceiverErr::RECEIVER_SUCCESS);
}

void VerilatorReceiver::close() noexcept
{
    if (mSockFd >= 0) {
        ::close(mSockFd);
        mSockFd = -1;
    }
}
