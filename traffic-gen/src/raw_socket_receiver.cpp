/*
 * SPDX-FileCopyrightText: 2025 Kebag-Logic (https://kebag-logic.com)
 * SPDX-FileCopyrightText: 2025 Alexandre Malki <alexandre.malki@kebag-logic.com>
 * SPDX-License-Identifier: MIT
 */

#include <raw_socket_receiver.h>

#include <cerrno>
#include <cstring>
#include <iostream>

#include <arpa/inet.h>
#include <net/ethernet.h>
#include <net/if.h>
#include <netpacket/packet.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

RawSocketReceiver::RawSocketReceiver(const std::string& ifName)
    : mIfName(ifName), mSockFd(-1)
{
}

RawSocketReceiver::~RawSocketReceiver()
{
    close();
}

ReceiverErr RawSocketReceiver::open()
{
    mSockFd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (mSockFd < 0) {
        std::cerr << "RawSocketReceiver: socket() failed: "
                  << std::strerror(errno) << "\n";
        return ReceiverErr(ReceiverErr::RECEIVER_ERR_OPEN_FAILED);
    }

    struct ifreq ifr = {};
    std::strncpy(ifr.ifr_name, mIfName.c_str(), IFNAMSIZ - 1);
    if (ioctl(mSockFd, SIOCGIFINDEX, &ifr) < 0) {
        std::cerr << "RawSocketReceiver: ioctl(SIOCGIFINDEX) on '"
                  << mIfName << "' failed: " << std::strerror(errno) << "\n";
        ::close(mSockFd);
        mSockFd = -1;
        return ReceiverErr(ReceiverErr::RECEIVER_ERR_OPEN_FAILED);
    }

    struct sockaddr_ll sll = {};
    sll.sll_family   = AF_PACKET;
    sll.sll_ifindex  = ifr.ifr_ifindex;
    sll.sll_protocol = htons(ETH_P_ALL);

    if (bind(mSockFd, reinterpret_cast<struct sockaddr*>(&sll),
             sizeof(sll)) < 0) {
        std::cerr << "RawSocketReceiver: bind() failed: "
                  << std::strerror(errno) << "\n";
        ::close(mSockFd);
        mSockFd = -1;
        return ReceiverErr(ReceiverErr::RECEIVER_ERR_OPEN_FAILED);
    }

    return ReceiverErr(ReceiverErr::RECEIVER_SUCCESS);
}

ReceiverErr RawSocketReceiver::receive(std::vector<uint8_t>& packet)
{
    if (mSockFd < 0) return ReceiverErr(ReceiverErr::RECEIVER_ERR_NOT_OPEN);

    packet.resize(MAX_FRAME_BYTES);
    const ssize_t n = recvfrom(mSockFd, packet.data(), packet.size(), 0,
                                nullptr, nullptr);
    if (n < 0) {
        if (errno == EINTR) {
            packet.clear();
            return ReceiverErr(ReceiverErr::RECEIVER_ERR_EOF);
        }
        std::cerr << "RawSocketReceiver: recvfrom() failed: "
                  << std::strerror(errno) << "\n";
        return ReceiverErr(ReceiverErr::RECEIVER_ERR_RECV_FAILED);
    }

    packet.resize(static_cast<size_t>(n));
    return ReceiverErr(ReceiverErr::RECEIVER_SUCCESS);
}

void RawSocketReceiver::close() noexcept
{
    if (mSockFd >= 0) {
        ::close(mSockFd);
        mSockFd = -1;
    }
}
