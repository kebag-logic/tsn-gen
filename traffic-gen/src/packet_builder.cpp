/*
 * SPDX-FileCopyrightText: 2025 Kebag-Logic (https://kebag-logic.com)
 * SPDX-FileCopyrightText: 2025 Alexandre Malki <alexandre.malki@kebag-logic.com>
 * SPDX-License-Identifier: MIT
 */

#include <packet_builder.h>

#include <cassert>
#include <iostream>
#include <limits>

PacketBuilder::PacketBuilder()
    : mRng(std::random_device{}())
{
}

void PacketBuilder::seed(uint64_t s)
{
    mRng.seed(s);
}

void PacketBuilder::appendBits(std::vector<uint8_t>& buf, size_t& bitOffset,
                                uint64_t value, uint32_t numBits)
{
    for (int i = static_cast<int>(numBits) - 1; i >= 0; --i) {
        const int bit = static_cast<int>((value >> i) & 1u);
        const size_t byteIdx = bitOffset / 8;
        const size_t bitInByte = 7 - (bitOffset % 8);  // MSB of byte first

        if (byteIdx >= buf.size()) {
            buf.push_back(0);
        }
        buf[byteIdx] |= static_cast<uint8_t>(bit << bitInByte);
        ++bitOffset;
    }
}

uint64_t PacketBuilder::pickValue(const Var& var)
{
    const auto& expected = var.getExpectedValues();

    if (!expected.empty()) {
        std::uniform_int_distribution<size_t> dist(0, expected.size() - 1);
        return static_cast<uint64_t>(expected[dist(mRng)]);
    }

    const uint32_t size = var.getSize();
    if (size == 0) {
        return 0;
    }
    const uint64_t maxVal = (size >= 64)
                            ? std::numeric_limits<uint64_t>::max()
                            : (1ULL << size) - 1ULL;
    std::uniform_int_distribution<uint64_t> dist(0, maxVal);
    return dist(mRng);
}

std::vector<uint8_t> PacketBuilder::build(const ProtocolInterface& iface,
                                           const Database<Var>& varDb)
{
    std::vector<uint8_t> buf;
    size_t bitOffset = 0;

    for (const std::string& ref : iface.getVarRefs()) {
        const Var* var = varDb.getElement(ref);
        if (var == nullptr) {
            std::cerr << "PacketBuilder: var_ref '" << ref
                      << "' not found in database, skipping\n";
            continue;
        }

        const uint32_t size = var->getSize();
        if (size == 0) {
            continue;
        }

        const uint64_t value = pickValue(*var);
        appendBits(buf, bitOffset, value, size);
    }

    return buf;
}
