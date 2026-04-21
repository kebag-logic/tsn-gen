/*
 * SPDX-FileCopyrightText: 2025 Kebag-Logic (https://kebag-logic.com)
 * SPDX-FileCopyrightText: 2025 Alexandre Malki <alexandre.malki@kebag-logic.com>
 * SPDX-License-Identifier: MIT
 */


 #pragma once

#include <cstdint>
#include <string>

#include <logic_module.h>

/**
 * @brief Reference logic module for the Ethernet II MAC frame.
 *
 *  Demux hook returns the upper-layer service name by inspecting
 *  ethertype. Until LayerContext exposes typed var accessors, this
 *  falls back to a hard-coded default of 1722 AVTP when no value
 *  is readable.
 *
 *  onEncode is a stub in v1: once LayerContext::setValue is backed
 *  by the serializer it will write 0x22F0 into the `ethertype` var
 *  when left unset.
 *
 *  onDecode reads `ethertype` and records the last value seen so
 *  tests can observe module activity without a full serializer.
 */
class EthernetFrameLogic final : public ILogicModule {
public:
    /** EtherType for IEEE 1722 / 1722.1 ATDECC. */
    static constexpr uint16_t kEthertype1722 = 0x22F0;

    void onEncode(LayerContext& ctx) override;
    void onDecode(LayerContext& ctx) override;
    std::string nextLayer(const LayerContext& ctx) const override;

    uint64_t getLastEthertypeSeen() const { return mLastEthertype; }
    int getEncodeCalls() const { return mEncodeCalls; }
    int getDecodeCalls() const { return mDecodeCalls; }

private:
    uint64_t mLastEthertype = 0;
    int mEncodeCalls = 0;
    int mDecodeCalls = 0;
};
