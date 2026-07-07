/*
 * SPDX-FileCopyrightText: 2026 Kebag-Logic (https://kebag-logic.com)
 * SPDX-FileCopyrightText: 2026 Alexandre Malki <alexandre.malki@kebag-logic.com>
 * SPDX-License-Identifier: MIT
 *
 * Golden tests for the logic-driven stack codec: the Eth + AVTP + AECP
 * ACQUIRE_ENTITY stack must produce semantically valid frames (derived
 * ethertype / subtype / control_data_length, command status forced to 0)
 * and decode back to the same fields.
 */

#include <tsn/session.h>
#include <tsn/stack.h>

using namespace tsn;

#include <gtest/gtest.h>

#include <map>
#include <memory>
#include <string>

#ifndef PROTOCOLS_ROOT_DIR
#error "PROTOCOLS_ROOT_DIR not defined"
#endif
#ifndef STACKS_ROOT_DIR
#error "STACKS_ROOT_DIR not defined"
#endif

namespace {

const std::string kStack =
    std::string(STACKS_ROOT_DIR) + "/aecp_acquire_entity.yaml";

/* Flatten a StackedPacket into service -> {field -> value}. */
std::map<std::string, std::map<std::string, uint64_t>>
byService(const Session::StackedPacket& pkt)
{
    std::map<std::string, std::map<std::string, uint64_t>> out;
    for (const auto& layer : pkt.layers) {
        auto& fields = out[layer.first];
        for (const auto& f : layer.second.fields) {
            fields[f.first] = f.second;
        }
    }
    return out;
}

} /* namespace */

class StackCodec : public ::testing::Test {
protected:
    void SetUp() override
    {
        session = std::make_unique<Session>(PROTOCOLS_ROOT_DIR);
        ASSERT_TRUE(session->parse());
        ASSERT_EQ(session->loadStack(kStack, stack).getErrorCode(),
                  StackBuilderErr::STACK_SUCCESS);
        ASSERT_EQ(stack->size(), 3u);
    }

    std::unique_ptr<Session> session;
    std::unique_ptr<Stack> stack;
};

TEST_F(StackCodec, FrameHasExpectedSize)
{
    session->seed(42);
    const Session::StackedPacket pkt = session->generateStack(*stack);
    /* 14 (Ethernet) + 2 (AVTP) + 39 (AECP ACQUIRE_ENTITY) = 55 bytes. */
    EXPECT_EQ(pkt.bytes.size(), 55u);
}

TEST_F(StackCodec, DerivedFieldsAreValid)
{
    session->seed(42);
    const Session::StackedPacket pkt = session->generateStack(*stack);
    const auto f = byService(pkt);

    /* Ethernet logic fills the ATDECC ethertype. */
    EXPECT_EQ(f.at("ethernet_mac_frame").at("ethertype"), 0x22F0u);
    /* AVTP logic derives the AECP subtype from the upper service. */
    EXPECT_EQ(f.at("avtp_control_header").at("subtype"), 0xFBu);
    EXPECT_EQ(f.at("avtp_control_header").at("sv"), 0u);
    EXPECT_EQ(f.at("avtp_control_header").at("version"), 0u);
    /* AECP logic derives control_data_length = 36 octets after the field. */
    EXPECT_EQ(f.at("atdecc_aecp_acquire_entity").at("control_data_length"),
              36u);
}

TEST_F(StackCodec, CommandStatusForcedToZero)
{
    /* Over many seeds, every command (even message_type) must carry
     * status 0; responses may carry any generated status. */
    for (uint64_t seed = 1; seed <= 200; ++seed) {
        session->seed(seed);
        const auto f = byService(session->generateStack(*stack));
        const auto& aecp = f.at("atdecc_aecp_acquire_entity");
        if (aecp.at("message_type") % 2 == 0) {
            EXPECT_EQ(aecp.at("status"), 0u)
                << "command with non-zero status at seed " << seed;
        }
    }
}

TEST_F(StackCodec, DecodeRoundTripsFields)
{
    session->seed(123);
    const Session::StackedPacket enc = session->generateStack(*stack);
    const Session::StackedPacket dec =
        session->decodeStack(*stack, enc.bytes);

    ASSERT_EQ(dec.bytes, enc.bytes);
    const auto encF = byService(enc);
    const auto decF = byService(dec);

    /* Every encoded field reappears with the same value on decode. */
    for (const auto& svc : encF) {
        for (const auto& field : svc.second) {
            ASSERT_TRUE(decF.count(svc.first));
            ASSERT_TRUE(decF.at(svc.first).count(field.first))
                << svc.first << "::" << field.first << " missing on decode";
            EXPECT_EQ(decF.at(svc.first).at(field.first), field.second)
                << svc.first << "::" << field.first;
        }
    }
}

TEST_F(StackCodec, DeterministicForFixedSeed)
{
    session->seed(999);
    const auto a = session->generateStack(*stack);
    session->seed(999);
    const auto b = session->generateStack(*stack);
    EXPECT_EQ(a.bytes, b.bytes);
}
