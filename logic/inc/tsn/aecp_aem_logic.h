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
 * @brief Logic for IEEE 1722.1 AECP AEM PDUs.
 *
 *  onEncode derives two fields the raw generator cannot get right on its
 *  own:
 *    - control_data_length = octets after the length field in this layer
 *      plus every byte stacked above it (IEEE 1722.1 §9.2.1.1.7);
 *    - status = 0 on commands (even message_type), left as generated on
 *      responses.
 *
 *  onDecode records the length seen; nextLayer reports the upper service
 *  (a control-data payload) when present.
 */
class AecpAemLogic final : public ILogicModule {
public:
    void onEncode(LayerContext& ctx) override;
    void onDecode(LayerContext& ctx) override;
    std::string nextLayer(const LayerContext& ctx) const override;

    uint64_t getLastControlDataLength() const { return mLastCdl; }

private:
    uint64_t mLastCdl = 0;
};

} /* namespace tsn */
