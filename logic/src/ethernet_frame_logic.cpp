/*
 * SPDX-FileCopyrightText: 2025 Kebag-Logic (https://kebag-logic.com)
 * SPDX-FileCopyrightText: 2025 Alexandre Malki <alexandre.malki@kebag-logic.com>
 * SPDX-License-Identifier: MIT
 */


 #include <tsn/ethernet_frame_logic.h>

#include <tsn/layer_context.h>
#include <tsn/logic_registry.h>

namespace tsn {

void EthernetFrameLogic::onEncode(LayerContext& ctx)
{
    ++mEncodeCalls;

    /* Fill the ethertype with the ATDECC value when the caller left it
     * unset (or zero). A YAML-fixed ethertype is kept as-is. */
    uint64_t current = 0;
    if (!ctx.getValue("ethertype", current) || current == 0) {
        ctx.setValue("ethertype", kEthertype1722);
    }
}

void EthernetFrameLogic::onDecode(LayerContext& ctx)
{
    ++mDecodeCalls;
    ctx.getValue("ethertype", mLastEthertype);
}

std::string EthernetFrameLogic::nextLayer(const LayerContext& ctx) const
{
    /* EtherType 0x22F0 covers the whole 1722 family; the streaming vs
     * control split lives in the AVTP subtype one layer up. In a bound
     * stack the concrete upper service is known, so predict it directly;
     * standalone (no upper) we fall back to the streaming default. */
    if (ctx.upper() != nullptr) {
        return ctx.upper()->getServiceName();
    }
    return "1722_avtp_common_stream";
}

REGISTER_LOGIC("ethernet_mac_frame", EthernetFrameLogic)

} /* namespace tsn */
