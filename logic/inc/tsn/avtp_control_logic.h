/*
 * SPDX-FileCopyrightText: 2026 Kebag-Logic (https://kebag-logic.com)
 * SPDX-FileCopyrightText: 2026 Alexandre Malki <alexandre.malki@kebag-logic.com>
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <cstdint>
#include <string>

#include <tsn/logic_module.h>

namespace tsn {

/**
 * @brief Logic for the 2-byte AVTP control header (IEEE 1722 §6.4).
 *
 *  onEncode selects the `subtype` from the upper layer's service:
 *  0xFB for an AECP payload, 0xFA for ADP, and forces the control
 *  invariants sv=0 / version=0 / reserved=0. nextLayer reports the
 *  upper service so a bound stack demuxes deterministically.
 */
class AvtpControlLogic final : public ILogicModule {
public:
    static constexpr uint8_t kSubtypeAecp = 0xFB;
    static constexpr uint8_t kSubtypeAdp  = 0xFA;

    void onEncode(LayerContext& ctx) override;
    void onDecode(LayerContext& ctx) override;
    std::string nextLayer(const LayerContext& ctx) const override;

    uint64_t getLastSubtypeSeen() const { return mLastSubtype; }

private:
    /** 0xFB / 0xFA implied by the upper service name, or 0 if unknown. */
    static uint8_t subtypeForUpper(const LayerContext& ctx);

    uint64_t mLastSubtype = 0;
};

} /* namespace tsn */
