/*
 * SPDX-FileCopyrightText: 2025 Kebag-Logic (https://kebag-logic.com)
 * SPDX-FileCopyrightText: 2025 Alexandre Malki <alexandre.malki@kebag-logic.com>
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <cstdint>
#include <vector>

#include <database.h>
#include <packet_builder.h>
#include <protocol_interface.h>
#include <var.h>

/**
 * @brief Decodes a raw byte buffer back into named field values.
 *
 * Symmetric counterpart to PacketBuilder: given a byte buffer and a
 * ProtocolInterface, extracts fields MSB-first in declaration order and
 * returns a BuiltPacket{bytes, fields} identical in structure to what
 * PacketBuilder would have produced for the same field values.
 *
 * Layer-sliced decoding (for --stack frames):
 *   The caller is responsible for passing a byte slice that starts at the
 *   correct offset for each layer.  Use layerBytes() to compute how many
 *   bytes a given interface consumes.
 */
class PacketDecoder {
public:
    /**
     * @brief Compute the byte length of one layer's contribution to a
     *        stacked frame: ceil(total_declared_bits / 8).
     */
    static size_t layerBytes(const ProtocolInterface& iface,
                             const Database<Var>& varDb);

    /**
     * @brief Decode @p bytes (starting at byte offset @p byteOffset) into
     *        the fields declared by @p iface.
     *
     * @param iface       Interface to decode against.
     * @param varDb       Global var database.
     * @param bytes       Full byte buffer (e.g. a stacked frame).
     * @param byteOffset  Byte offset within @p bytes where this layer starts.
     * @return BuiltPacket whose 'bytes' is the extracted layer slice and
     *         whose 'fields' holds short-name → decoded value pairs.
     */
    PacketBuilder::BuiltPacket decode(const ProtocolInterface& iface,
                                      const Database<Var>& varDb,
                                      const std::vector<uint8_t>& bytes,
                                      size_t byteOffset = 0);

private:
    /** Extract numBits MSB-first from buf at bitOffset (in/out). */
    static uint64_t extractBits(const std::vector<uint8_t>& buf,
                                 size_t& bitOffset,
                                 uint32_t numBits);
};
