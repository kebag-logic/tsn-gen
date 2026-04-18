/*
 * SPDX-FileCopyrightText: 2025 Kebag-Logic (https://kebag-logic.com)
 * SPDX-FileCopyrightText: 2025 Alexandre Malki <alexandre.malki@kebag-logic.com>
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <cstddef>
#include <cstdint>

/**
 * @brief AXI-Stream beat (64-bit TDATA width).
 *
 * Shared between the socket-based Verilator transport (VerilatorSender /
 * VerilatorReceiver) and the SystemC bridge (AxisBridge).
 *
 * Field layout:
 *   tdata — 8 payload bytes, first packet byte in bits [7:0] (little-endian)
 *   tkeep — byte-enable mask: bit i set means byte i of tdata is valid
 *   tlast — 1 on the last beat of a packet, 0 otherwise
 */
struct AxiStreamBeat {
    uint64_t tdata;
    uint8_t  tkeep;
    uint8_t  tlast;
} __attribute__((packed));

static_assert(sizeof(AxiStreamBeat) == 10, "AxiStreamBeat must be exactly 10 bytes");

static constexpr size_t AXIS_TDATA_BYTES = 8;
