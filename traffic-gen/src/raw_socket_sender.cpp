/*
 * SPDX-FileCopyrightText: 2025 Kebag-Logic (https://kebag-logic.com)
 * SPDX-FileCopyrightText: 2025 Alexandre Malki <alexandre.malki@kebag-logic.com>
 * SPDX-License-Identifier: MIT
 */

#include <raw_socket_sender.h>

#include <cerrno>
#include <cstring>
#include <iostream>

#include <arpa/inet.h>
#include <net/if.h>
#include <netpacket/packet.h>
#include <net/ethernet.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

RawSocketSender::RawSocketSender(const std::string& ifName)
    : mIfName(ifName), mSockFd(-1), mIfIndex(-1)
{
}

RawSocketSender::~RawSocketSender()
{
    close();
}

const SenderErr RawSocketSender::open()
{
    mSockFd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (mSockFd < 0) {
        std::cerr << "RawSocketSender: socket() failed: "
                  << std::strerror(errno) << "\n";
        return SenderErr(SenderErr::SENDER_ERR_OPEN_FAILED);
    }

    struct ifreq ifr = {};
    std::strncpy(ifr.ifr_name, mIfName.c_str(), IFNAMSIZ - 1);
    if (ioctl(mSockFd, SIOCGIFINDEX, &ifr) < 0) {
        std::cerr << "RawSocketSender: ioctl(SIOCGIFINDEX) on '"
                  << mIfName << "' failed: " << std::strerror(errno) << "\n";
        ::close(mSockFd);
        mSockFd = -1;
        return SenderErr(SenderErr::SENDER_ERR_OPEN_FAILED);
    }
    mIfIndex = ifr.ifr_ifindex;

    struct sockaddr_ll sll = {};
    sll.sll_family   = AF_PACKET;
    sll.sll_ifindex  = mIfIndex;
    sll.sll_protocol = htons(ETH_P_ALL);

    if (bind(mSockFd, reinterpret_cast<struct sockaddr*>(&sll),
             sizeof(sll)) < 0) {
        std::cerr << "RawSocketSender: bind() failed: "
                  << std::strerror(errno) << "\n";
        ::close(mSockFd);
        mSockFd = -1;
        return SenderErr(SenderErr::SENDER_ERR_OPEN_FAILED);
    }

    return SenderErr(SenderErr::SENDER_SUCCESS);
}

const SenderErr RawSocketSender::send(const std::vector<uint8_t>& packet)
{
    if (mSockFd < 0) {
        return SenderErr(SenderErr::SENDER_ERR_NOT_OPEN);
    }

    struct sockaddr_ll sll = {};
    sll.sll_family  = AF_PACKET;
    sll.sll_ifindex = mIfIndex;

    const ssize_t sent = sendto(mSockFd,
                                packet.data(),
                                packet.size(),
                                0,
                                reinterpret_cast<struct sockaddr*>(&sll),
                                sizeof(sll));
    if (sent < 0) {
        std::cerr << "RawSocketSender: sendto() failed: "
                  << std::strerror(errno) << "\n";
        return SenderErr(SenderErr::SENDER_ERR_SEND_FAILED);
    }

    return SenderErr(SenderErr::SENDER_SUCCESS);
}

void RawSocketSender::close() noexcept
{
    if (mSockFd >= 0) {
        ::close(mSockFd);
        mSockFd  = -1;
        mIfIndex = -1;
    }
}
