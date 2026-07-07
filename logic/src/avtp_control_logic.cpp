/*
 * SPDX-FileCopyrightText: 2026 Kebag-Logic (https://kebag-logic.com)
 * SPDX-FileCopyrightText: 2026 Alexandre Malki <alexandre.malki@kebag-logic.com>
 * SPDX-License-Identifier: MIT
 */

#include <tsn/avtp_control_logic.h>

#include <tsn/layer_context.h>
#include <tsn/logic_registry.h>

namespace tsn {

uint8_t AvtpControlLogic::subtypeForUpper(const LayerContext& ctx)
{
    const LayerContext* up = ctx.upper();
    if (up == nullptr) {
        return 0;
    }
    const std::string& svc = up->getServiceName();
    /* Match the ATDECC service families by name prefix. */
    if (svc.rfind("atdecc_aecp", 0) == 0) return kSubtypeAecp;
    if (svc.rfind("atdecc_adp", 0) == 0)  return kSubtypeAdp;
    return 0;
}

void AvtpControlLogic::onEncode(LayerContext& ctx)
{
    const uint8_t subtype = subtypeForUpper(ctx);
    if (subtype != 0) {
        ctx.setValue("subtype", subtype);
    }
    /* Control-message invariants (harmless if the fields are absent). */
    ctx.setValue("sv", 0);
    ctx.setValue("version", 0);
    ctx.setValue("reserved", 0);
}

void AvtpControlLogic::onDecode(LayerContext& ctx)
{
    ctx.getValue("subtype", mLastSubtype);
}

std::string AvtpControlLogic::nextLayer(const LayerContext& ctx) const
{
    /* The concrete upper service (e.g. atdecc_aecp_acquire_entity) is the
     * demux target in a bound stack; the subtype byte only identifies the
     * family. */
    if (ctx.upper() != nullptr) {
        return ctx.upper()->getServiceName();
    }
    return {};
}

REGISTER_LOGIC("avtp_control", AvtpControlLogic)

} /* namespace tsn */
