/*
 * SPDX-FileCopyrightText: 2025 Kebag-Logic (https://kebag-logic.com)
 * SPDX-FileCopyrightText: 2025 Alexandre Malki <alexandre.malki@kebag-logic.com>
 * SPDX-License-Identifier: MIT
 */

#include <gtest/gtest.h>

#include <packet_builder.h>
#include <protocol_interface.h>
#include <protocol_parser.h>
#include <var.h>

#include <cstdint>
#include <string>
#include <vector>

#ifndef TRAFFIC_GEN_TESTS_RES_PATH
#error "TRAFFIC_GEN_TESTS_RES_PATH not defined"
#endif

/* ------------------------------------------------------------------ */
/*  Constants                                                          */
/* ------------------------------------------------------------------ */

static const std::string kAdpPath =
    TRAFFIC_GEN_TESTS_RES_PATH "/0020_adp_protocol_test/";

static const std::string kSvc = "atdecc_adp_service";

static const std::string kIfData =
    "atdecc_adp_service::ADP_INTERFACE::ADP_IF_DATA";

/* ------------------------------------------------------------------ */
/*  Helpers                                                            */
/* ------------------------------------------------------------------ */

/* Reconstruct an N-bit MSB-first field from a standalone packet buffer. */
static uint64_t extractField(const std::vector<uint8_t>& pkt, uint32_t bits)
{
    uint64_t value = 0;
    for (size_t i = 0; i < pkt.size(); ++i) {
        value = (value << 8) | pkt[i];
    }
    /* Drop padding bits at the LSB end. */
    const uint32_t padBits = static_cast<uint32_t>(pkt.size() * 8) - bits;
    return value >> padBits;
}

/* Build a standalone single-var interface and return one generated value. */
static uint64_t buildSingleVar(PacketBuilder& builder,
                                const ProtocolParser& parser,
                                const std::string& qualifiedVarName)
{
    const Var* v = parser.getVarDatabase().getElement(qualifiedVarName);
    if (!v) return ~uint64_t{0};

    ProtocolInterface iface("test::E::IF", ProtocolInterface::IN,
                            {qualifiedVarName});
    auto pkt = builder.build(iface, parser.getVarDatabase());
    return extractField(pkt, v->getSize());
}

/* ------------------------------------------------------------------ */
/*  Test fixture                                                        */
/* ------------------------------------------------------------------ */

class AdpFuzzTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        ASSERT_EQ(mParser.parse().getErrorCode(),
                  ProtocolParserErr::PROTOPARSER_SUCCESS);
    }

    ProtocolParser mParser{kAdpPath};
};

/* ------------------------------------------------------------------ */
/*  1. Parser: var database                                            */
/* ------------------------------------------------------------------ */

TEST_F(AdpFuzzTest, VarCount)
{
    /* 20 PDU fields declared in adp.yaml */
    EXPECT_EQ(mParser.getVarDatabase().size(), 20u);
}

TEST_F(AdpFuzzTest, MessageTypeConstraints)
{
    const Var* v = mParser.getVarDatabase().getElement(kSvc + "::message_type");
    ASSERT_NE(v, nullptr);
    EXPECT_EQ(v->getSize(), 4u);
    ASSERT_EQ(v->getExpectedValues().size(), 3u);
    EXPECT_EQ(v->getExpectedValues()[0], 0);
    EXPECT_EQ(v->getExpectedValues()[1], 1);
    EXPECT_EQ(v->getExpectedValues()[2], 2);
}

TEST_F(AdpFuzzTest, ValidTimeRangeConstraint)
{
    const Var* v = mParser.getVarDatabase().getElement(kSvc + "::valid_time");
    ASSERT_NE(v, nullptr);
    EXPECT_EQ(v->getSize(), 5u);
    EXPECT_TRUE(v->hasRange());
    EXPECT_EQ(v->getRange().min, 1);
    EXPECT_EQ(v->getRange().max, 31);
    EXPECT_TRUE(v->getExpectedValues().empty());
}

TEST_F(AdpFuzzTest, ControlDataLengthFixed)
{
    const Var* v =
        mParser.getVarDatabase().getElement(kSvc + "::control_data_length");
    ASSERT_NE(v, nullptr);
    EXPECT_EQ(v->getSize(), 11u);
    ASSERT_EQ(v->getExpectedValues().size(), 1u);
    EXPECT_EQ(v->getExpectedValues()[0], 56);
}

TEST_F(AdpFuzzTest, TalkerCapabilitiesMask)
{
    const Var* v =
        mParser.getVarDatabase().getElement(kSvc + "::talker_capabilities");
    ASSERT_NE(v, nullptr);
    EXPECT_EQ(v->getSize(), 16u);
    EXPECT_TRUE(v->hasMask());
    EXPECT_EQ(v->getMask(), uint64_t{0x803F});
}

TEST_F(AdpFuzzTest, ListenerCapabilitiesMask)
{
    const Var* v =
        mParser.getVarDatabase().getElement(kSvc + "::listener_capabilities");
    ASSERT_NE(v, nullptr);
    EXPECT_TRUE(v->hasMask());
    EXPECT_EQ(v->getMask(), uint64_t{0x803F});
}

TEST_F(AdpFuzzTest, ControllerCapabilitiesMask)
{
    const Var* v =
        mParser.getVarDatabase().getElement(kSvc + "::controller_capabilities");
    ASSERT_NE(v, nullptr);
    EXPECT_EQ(v->getSize(), 32u);
    EXPECT_TRUE(v->hasMask());
    EXPECT_EQ(v->getMask(), uint64_t{0x8000});
}

TEST_F(AdpFuzzTest, ReservedFieldsAreFixed)
{
    const Var* r0 = mParser.getVarDatabase().getElement(kSvc + "::reserved0");
    const Var* r1 = mParser.getVarDatabase().getElement(kSvc + "::reserved1");
    ASSERT_NE(r0, nullptr);
    ASSERT_NE(r1, nullptr);
    ASSERT_EQ(r0->getExpectedValues().size(), 1u);
    EXPECT_EQ(r0->getExpectedValues()[0], 0);
    ASSERT_EQ(r1->getExpectedValues().size(), 1u);
    EXPECT_EQ(r1->getExpectedValues()[0], 0);
}

/* ------------------------------------------------------------------ */
/*  2. Parser: interface database                                      */
/* ------------------------------------------------------------------ */

TEST_F(AdpFuzzTest, InterfaceCount)
{
    /* ADP_IF_DATA (in), ADP_DLL_INFO (in), ADP_ENT_IF_DATA (out) */
    EXPECT_EQ(mParser.getInterfaceDatabase().size(), 3u);
}

TEST_F(AdpFuzzTest, IfDataDirection)
{
    const ProtocolInterface* iface =
        mParser.getInterfaceDatabase().getElement(kIfData);
    ASSERT_NE(iface, nullptr);
    EXPECT_EQ(iface->getDirection(), ProtocolInterface::IN);
}

TEST_F(AdpFuzzTest, IfDataHas20VarRefs)
{
    const ProtocolInterface* iface =
        mParser.getInterfaceDatabase().getElement(kIfData);
    ASSERT_NE(iface, nullptr);
    EXPECT_EQ(iface->getVarRefs().size(), 20u);
}

TEST_F(AdpFuzzTest, IfDataFirstVarRefIsMessageType)
{
    const ProtocolInterface* iface =
        mParser.getInterfaceDatabase().getElement(kIfData);
    ASSERT_NE(iface, nullptr);
    ASSERT_FALSE(iface->getVarRefs().empty());
    EXPECT_EQ(iface->getVarRefs().front(), kSvc + "::message_type");
}

TEST_F(AdpFuzzTest, IfDataLastVarRefIsReserved1)
{
    const ProtocolInterface* iface =
        mParser.getInterfaceDatabase().getElement(kIfData);
    ASSERT_NE(iface, nullptr);
    ASSERT_FALSE(iface->getVarRefs().empty());
    EXPECT_EQ(iface->getVarRefs().back(), kSvc + "::reserved1");
}

/* ------------------------------------------------------------------ */
/*  3. PacketBuilder: packet size                                      */
/* ------------------------------------------------------------------ */

TEST_F(AdpFuzzTest, FullAdpPacketIs67Bytes)
{
    /*
     * 20 fields totalling 532 bits.  ceil(532/8) = 67 bytes (4 pad bits).
     * The bit-packer should produce exactly 67 bytes for any seed.
     */
    const ProtocolInterface* iface =
        mParser.getInterfaceDatabase().getElement(kIfData);
    ASSERT_NE(iface, nullptr);

    PacketBuilder builder;
    builder.seed(0);
    for (int trial = 0; trial < 50; ++trial) {
        const auto pkt = builder.build(*iface, mParser.getVarDatabase());
        EXPECT_EQ(pkt.size(), 67u) << "trial " << trial;
    }
}

/* ------------------------------------------------------------------ */
/*  4. PacketBuilder: constraint compliance (single-var harness)      */
/* ------------------------------------------------------------------ */

TEST_F(AdpFuzzTest, MessageTypeAlwaysValid)
{
    PacketBuilder builder;
    builder.seed(0);
    for (int trial = 0; trial < 500; ++trial) {
        const uint64_t v =
            buildSingleVar(builder, mParser, kSvc + "::message_type");
        EXPECT_TRUE(v == 0 || v == 1 || v == 2)
            << "trial " << trial << " value=" << v;
    }
}

TEST_F(AdpFuzzTest, ValidTimeAlwaysInRange)
{
    PacketBuilder builder;
    builder.seed(1);
    for (int trial = 0; trial < 500; ++trial) {
        const uint64_t v =
            buildSingleVar(builder, mParser, kSvc + "::valid_time");
        EXPECT_GE(v, uint64_t{1}) << "trial " << trial;
        EXPECT_LE(v, uint64_t{31}) << "trial " << trial;
    }
}

TEST_F(AdpFuzzTest, ControlDataLengthAlways56)
{
    PacketBuilder builder;
    builder.seed(2);
    for (int trial = 0; trial < 100; ++trial) {
        const uint64_t v =
            buildSingleVar(builder, mParser, kSvc + "::control_data_length");
        EXPECT_EQ(v, uint64_t{56}) << "trial " << trial;
    }
}

TEST_F(AdpFuzzTest, TalkerCapsRespectMask)
{
    constexpr uint64_t kMask = 0x803F;
    PacketBuilder builder;
    builder.seed(3);
    for (int trial = 0; trial < 500; ++trial) {
        const uint64_t v =
            buildSingleVar(builder, mParser, kSvc + "::talker_capabilities");
        EXPECT_EQ(v & ~kMask, uint64_t{0})
            << "trial " << trial << " value=0x" << std::hex << v;
    }
}

TEST_F(AdpFuzzTest, ListenerCapsRespectMask)
{
    constexpr uint64_t kMask = 0x803F;
    PacketBuilder builder;
    builder.seed(4);
    for (int trial = 0; trial < 500; ++trial) {
        const uint64_t v =
            buildSingleVar(builder, mParser, kSvc + "::listener_capabilities");
        EXPECT_EQ(v & ~kMask, uint64_t{0})
            << "trial " << trial << " value=0x" << std::hex << v;
    }
}

TEST_F(AdpFuzzTest, ControllerCapsRespectMask)
{
    constexpr uint64_t kMask = 0x8000;
    PacketBuilder builder;
    builder.seed(5);
    for (int trial = 0; trial < 500; ++trial) {
        const uint64_t v =
            buildSingleVar(builder, mParser, kSvc + "::controller_capabilities");
        EXPECT_EQ(v & ~kMask, uint64_t{0})
            << "trial " << trial << " value=0x" << std::hex << v;
    }
}

TEST_F(AdpFuzzTest, ReservedFieldsAlwaysZero)
{
    PacketBuilder builder;
    builder.seed(6);
    for (int trial = 0; trial < 100; ++trial) {
        EXPECT_EQ(buildSingleVar(builder, mParser, kSvc + "::reserved0"),
                  uint64_t{0})
            << "trial " << trial;
        EXPECT_EQ(buildSingleVar(builder, mParser, kSvc + "::reserved1"),
                  uint64_t{0})
            << "trial " << trial;
    }
}

TEST_F(AdpFuzzTest, AvailableIndexAlwaysValid)
{
    PacketBuilder builder;
    builder.seed(7);
    for (int trial = 0; trial < 200; ++trial) {
        const uint64_t v =
            buildSingleVar(builder, mParser, kSvc + "::available_index");
        EXPECT_TRUE(v == 0 || v == 1)
            << "trial " << trial << " value=" << v;
    }
}
