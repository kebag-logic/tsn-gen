/*
 * SPDX-FileCopyrightText: 2025 Kebag-Logic (https://kebag-logic.com)
 * SPDX-FileCopyrightText: 2025 Alexandre Malki <alexandre.malki@kebag-logic.com>
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <cstdint>
#include <random>
#include <vector>

#include <database.h>
#include <protocol_interface.h>
#include <var.h>

/**
 * @brief Builds a raw packet byte buffer from a parsed protocol interface.
 *
 * For each var referenced by the interface (in declaration order):
 *   - If the Var has expected values, one is chosen uniformly at random.
 *   - Otherwise a random value in [0, 2^size - 1] is generated.
 *
 * Fields are bit-packed MSB-first into a byte array that is padded to the
 * next byte boundary.  Example with two fields of 4 and 3 bits:
 *
 *   field0[3:0]  field1[2:0]  pad[0]
 *   → 1 byte total
 *
 * Vars whose qualified name is not found in the database (e.g. external
 * var_refs like "system::time::currentTime") are silently skipped.
 */
class PacketBuilder {
public:
    PacketBuilder();

    /**
     * @brief Build one packet for the given interface.
     *
     * @param iface   Parsed interface (carries the ordered var_ref list).
     * @param varDb   Global var database populated by ProtocolParser.
     * @return Byte buffer; empty if the interface has no resolvable var_refs.
     */
    std::vector<uint8_t> build(const ProtocolInterface& iface,
                               const Database<Var>& varDb);

    /**
     * @brief Seed the internal PRNG (useful for deterministic tests).
     */
    void seed(uint64_t s);

private:
    /** Pick one value for a single var according to its constraints. */
    uint64_t pickValue(const Var& var);

    /**
     * @brief Append numBits of value (MSB first) to buf at the given bit offset.
     *        Grows buf as needed.
     */
    static void appendBits(std::vector<uint8_t>& buf, size_t& bitOffset,
                           uint64_t value, uint32_t numBits);

    std::mt19937_64 mRng;
};
