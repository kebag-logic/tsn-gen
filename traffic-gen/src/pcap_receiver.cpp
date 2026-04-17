/*
 * SPDX-FileCopyrightText: 2025 Kebag-Logic (https://kebag-logic.com)
 * SPDX-FileCopyrightText: 2025 Alexandre Malki <alexandre.malki@kebag-logic.com>
 * SPDX-License-Identifier: MIT
 */

#include <pcap_receiver.h>

#include <cerrno>
#include <cstring>
#include <iostream>

#include <fcntl.h>
#include <unistd.h>

namespace {

constexpr uint32_t PCAP_MAGIC_LE = 0xA1B2C3D4u;
constexpr uint32_t PCAP_MAGIC_BE = 0xD4C3B2A1u;

uint32_t swap32(uint32_t v)
{
    return ((v & 0xFF000000u) >> 24) | ((v & 0x00FF0000u) >>  8)
         | ((v & 0x0000FF00u) <<  8) | ((v & 0x000000FFu) << 24);
}

bool readAll(int fd, void* buf, size_t len)
{
    uint8_t* p = reinterpret_cast<uint8_t*>(buf);
    size_t rem = len;
    while (rem > 0) {
        const ssize_t n = ::read(fd, p, rem);
        if (n == 0) return false;
        if (n < 0) {
            if (errno == EINTR) continue;
            return false;
        }
        p   += n;
        rem -= static_cast<size_t>(n);
    }
    return true;
}

} // namespace

PcapReceiver::PcapReceiver(const std::string& filePath)
    : mFilePath(filePath), mFd(-1), mSwap(false)
{
}

PcapReceiver::~PcapReceiver()
{
    close();
}

ReceiverErr PcapReceiver::open()
{
    mFd = ::open(mFilePath.c_str(), O_RDONLY);
    if (mFd < 0) {
        std::cerr << "PcapReceiver: open('" << mFilePath
                  << "') failed: " << std::strerror(errno) << "\n";
        return ReceiverErr(ReceiverErr::RECEIVER_ERR_OPEN_FAILED);
    }

    /* Read and validate global header (24 bytes). */
    struct { uint32_t magic, ver_maj, ver_min, thiszone,
                      sigfigs, snaplen, network; } gh{};
    if (!readAll(mFd, &gh, 24)) {
        std::cerr << "PcapReceiver: file too short for global header\n";
        ::close(mFd);
        mFd = -1;
        return ReceiverErr(ReceiverErr::RECEIVER_ERR_OPEN_FAILED);
    }

    if (gh.magic == PCAP_MAGIC_BE) {
        mSwap = true;
    } else if (gh.magic != PCAP_MAGIC_LE) {
        std::cerr << "PcapReceiver: not a pcap file (magic="
                  << std::hex << gh.magic << ")\n";
        ::close(mFd);
        mFd = -1;
        return ReceiverErr(ReceiverErr::RECEIVER_ERR_OPEN_FAILED);
    }

    return ReceiverErr(ReceiverErr::RECEIVER_SUCCESS);
}

ReceiverErr PcapReceiver::receive(std::vector<uint8_t>& packet)
{
    if (mFd < 0) return ReceiverErr(ReceiverErr::RECEIVER_ERR_NOT_OPEN);

    /* Per-packet header: ts_sec(4) ts_usec(4) incl_len(4) orig_len(4). */
    uint32_t ph[4];
    if (!readAll(mFd, ph, 16)) {
        return ReceiverErr(ReceiverErr::RECEIVER_ERR_EOF);
    }

    uint32_t inclLen = mSwap ? swap32(ph[2]) : ph[2];
    if (inclLen == 0) {
        packet.clear();
        return ReceiverErr(ReceiverErr::RECEIVER_SUCCESS);
    }

    packet.resize(inclLen);
    if (!readAll(mFd, packet.data(), inclLen)) {
        std::cerr << "PcapReceiver: truncated packet record\n";
        return ReceiverErr(ReceiverErr::RECEIVER_ERR_RECV_FAILED);
    }

    return ReceiverErr(ReceiverErr::RECEIVER_SUCCESS);
}

void PcapReceiver::close() noexcept
{
    if (mFd >= 0) {
        ::close(mFd);
        mFd = -1;
    }
}
