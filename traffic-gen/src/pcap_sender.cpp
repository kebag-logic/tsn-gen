/*
 * SPDX-FileCopyrightText: 2025 Kebag-Logic (https://kebag-logic.com)
 * SPDX-FileCopyrightText: 2025 Alexandre Malki <alexandre.malki@kebag-logic.com>
 * SPDX-License-Identifier: MIT
 */

#include <pcap_sender.h>

#include <cstring>
#include <ctime>
#include <fcntl.h>
#include <unistd.h>

/* ------------------------------------------------------------------ */
/*  Raw pcap format helpers (no libpcap required)                     */
/* ------------------------------------------------------------------ */

namespace {

/* pcap global file header — written once on open(). */
struct PcapGlobalHeader {
    uint32_t magic_number  = 0xA1B2C3D4u;  /* native byte-order magic */
    uint16_t version_major = 2u;
    uint16_t version_minor = 4u;
    int32_t  thiszone      = 0;             /* GMT */
    uint32_t sigfigs       = 0;
    uint32_t snaplen       = 65535u;
    uint32_t network       = 1u;            /* LINKTYPE_ETHERNET */
};

/* pcap per-packet record header — written before each packet. */
struct PcapPacketHeader {
    uint32_t ts_sec;
    uint32_t ts_usec;
    uint32_t incl_len;
    uint32_t orig_len;
};

bool writeAll(int fd, const void* data, size_t len)
{
    const uint8_t* p = static_cast<const uint8_t*>(data);
    while (len > 0) {
        const ssize_t n = ::write(fd, p, len);
        if (n <= 0) return false;
        p   += n;
        len -= static_cast<size_t>(n);
    }
    return true;
}

} // namespace

/* ------------------------------------------------------------------ */
/*  PcapSender                                                         */
/* ------------------------------------------------------------------ */

PcapSender::PcapSender(const std::string& filePath)
    : mFilePath(filePath), mFd(-1)
{
}

PcapSender::~PcapSender()
{
    close();
}

const SenderErr PcapSender::open()
{
    mFd = ::open(mFilePath.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (mFd < 0) {
        return SenderErr(SenderErr::SENDER_ERR_OPEN_FAILED);
    }

    PcapGlobalHeader hdr{};
    if (!writeAll(mFd, &hdr, sizeof(hdr))) {
        ::close(mFd);
        mFd = -1;
        return SenderErr(SenderErr::SENDER_ERR_OPEN_FAILED);
    }

    return SenderErr(SenderErr::SENDER_SUCCESS);
}

const SenderErr PcapSender::send(const std::vector<uint8_t>& packet)
{
    if (mFd < 0) {
        return SenderErr(SenderErr::SENDER_ERR_NOT_OPEN);
    }

    struct timespec ts{};
    clock_gettime(CLOCK_REALTIME, &ts);

    PcapPacketHeader phdr{};
    phdr.ts_sec  = static_cast<uint32_t>(ts.tv_sec);
    phdr.ts_usec = static_cast<uint32_t>(ts.tv_nsec / 1000u);
    phdr.incl_len = static_cast<uint32_t>(packet.size());
    phdr.orig_len = phdr.incl_len;

    if (!writeAll(mFd, &phdr, sizeof(phdr)) ||
        !writeAll(mFd, packet.data(), packet.size())) {
        return SenderErr(SenderErr::SENDER_ERR_SEND_FAILED);
    }

    return SenderErr(SenderErr::SENDER_SUCCESS);
}

void PcapSender::close() noexcept
{
    if (mFd >= 0) {
        ::close(mFd);
        mFd = -1;
    }
}
