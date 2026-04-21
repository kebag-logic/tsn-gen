/*
 * SPDX-FileCopyrightText: 2025 Kebag-Logic (https://kebag-logic.com)
 * SPDX-FileCopyrightText: 2025 Alexandre Malki <alexandre.malki@kebag-logic.com>
 * SPDX-License-Identifier: MIT
 */


 #include <ethernet_frame_logic.h>

#include <layer_context.h>
#include <logic_registry.h>

void EthernetFrameLogic::onEncode(LayerContext& ctx)
{
    ++mEncodeCalls;

    /* When the serializer backs LayerContext::setValue, unwritten
     * ethertype fields get the default 1722 value here. Until then
     * this call is a best-effort no-op. */
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
    uint64_t eth = 0;
    if (ctx.getValue("ethertype", eth) && eth == kEthertype1722) {
        return "1722_avtp_common_stream";
    }
    /* Default on unknown / unreadable ethertype so static stacks still
     * chain deterministically during the v1 LayerContext stubs. */
    return "1722_avtp_common_stream";
}

REGISTER_LOGIC("ethernet_mac_frame", EthernetFrameLogic)
