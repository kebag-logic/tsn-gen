/*
 * SPDX-FileCopyrightText: 2025 Kebag-Logic (https://kebag-logic.com)
 * SPDX-FileCopyrightText: 2025 Alexandre Malki <alexandre.malki@kebag-logic.com>
 * SPDX-License-Identifier: MIT
 */

#include <packet_decoder.h>

#include <iostream>

size_t PacketDecoder::layerBytes(const ProtocolInterface& iface,
                                  const Database<Var>& varDb)
{
    size_t totalBits = 0;
    for (const std::string& ref : iface.getVarRefs()) {
        const Var* var = varDb.getElement(ref);
        if (var) totalBits += var->getSize();
    }
    return (totalBits + 7) / 8;
}

uint64_t PacketDecoder::extractBits(const std::vector<uint8_t>& buf,
                                     size_t& bitOffset,
                                     uint32_t numBits)
{
    uint64_t value = 0;
    for (int i = static_cast<int>(numBits) - 1; i >= 0; --i) {
        const size_t byteIdx   = bitOffset / 8;
        const size_t bitInByte = 7 - (bitOffset % 8);
        if (byteIdx < buf.size()) {
            const int bit =
                static_cast<int>((buf[byteIdx] >> bitInByte) & 1u);
            value |= static_cast<uint64_t>(bit) << i;
        }
        ++bitOffset;
    }
    return value;
}

PacketBuilder::BuiltPacket PacketDecoder::decode(
    const ProtocolInterface& iface,
    const Database<Var>& varDb,
    const std::vector<uint8_t>& bytes,
    size_t byteOffset)
{
    PacketBuilder::BuiltPacket result;
    size_t bitOffset = byteOffset * 8;

    for (const std::string& ref : iface.getVarRefs()) {
        const Var* var = varDb.getElement(ref);
        if (var == nullptr) {
            std::cerr << "PacketDecoder: var_ref '" << ref
                      << "' not found in database, skipping\n";
            continue;
        }

        const uint32_t size = var->getSize();
        if (size == 0) continue;

        const uint64_t value = extractBits(bytes, bitOffset, size);

        const auto pos = ref.rfind("::");
        const std::string shortName =
            (pos != std::string::npos) ? ref.substr(pos + 2) : ref;

        result.fields.emplace_back(shortName, value);
    }

    /* Populate bytes: the slice of the input buffer used by this layer. */
    const size_t layerLen = layerBytes(iface, varDb);
    const size_t end = std::min(byteOffset + layerLen, bytes.size());
    if (byteOffset < end) {
        result.bytes.assign(bytes.begin() + byteOffset,
                            bytes.begin() + end);
    }

    return result;
}
