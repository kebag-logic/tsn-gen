/*
 * SPDX-FileCopyrightText: 2026 Kebag-Logic (https://kebag-logic.com)
 * SPDX-FileCopyrightText: 2026 Alexandre Malki <alexandre.malki@kebag-logic.com>
 * SPDX-License-Identifier: MIT
 */

#include <tsn/aecp_aem_logic.h>

#include <cstddef>

#include <tsn/layer_context.h>
#include <tsn/logic_registry.h>

namespace tsn {

namespace {
constexpr char kControlDataLength[] = "control_data_length";
constexpr char kMessageType[]       = "message_type";
constexpr char kStatus[]            = "status";
}

void AecpAemLogic::onEncode(LayerContext& ctx)
{
    /* control_data_length counts the octets following the field within
     * the AECPDU, plus the bytes of any control-data payload stacked
     * above this layer. */
    size_t after = 0;
    if (ctx.bytesAfter(kControlDataLength, after)) {
        const size_t cdl = after + ctx.bytesAbove();
        ctx.setValue(kControlDataLength, static_cast<uint64_t>(cdl));
    }

    /* A command (even message_type) carries status 0; a controller must
     * not leak a random status code onto the wire. Responses keep the
     * generated value. */
    uint64_t messageType = 0;
    if (ctx.getValue(kMessageType, messageType) && (messageType % 2) == 0) {
        ctx.setValue(kStatus, 0);
    }
}

void AecpAemLogic::onDecode(LayerContext& ctx)
{
    ctx.getValue(kControlDataLength, mLastCdl);
}

std::string AecpAemLogic::nextLayer(const LayerContext& ctx) const
{
    if (ctx.upper() != nullptr) {
        return ctx.upper()->getServiceName();
    }
    return {};
}

REGISTER_LOGIC("aecp_aem", AecpAemLogic)

} /* namespace tsn */
