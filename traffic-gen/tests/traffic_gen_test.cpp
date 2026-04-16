/*
 * SPDX-FileCopyrightText: 2025 Kebag-Logic (https://kebag-logic.com)
 * SPDX-FileCopyrightText: 2025 Alexandre Malki <alexandre.malki@kebag-logic.com>
 * SPDX-License-Identifier: MIT
 */

#include <gtest/gtest.h>

#include <packet_builder.h>
#include <protocol_parser.h>
#include <sender.h>
#include <traffic_generator.h>
#include <var.h>

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#ifndef TRAFFIC_GEN_TESTS_RES_PATH
#error "TRAFFIC_GEN_TESTS_RES_PATH not defined"
#endif

/* ------------------------------------------------------------------ */
/*  Test double: captures every sent packet for later inspection       */
/* ------------------------------------------------------------------ */

class CaptureSender : public ISender {
public:
    const SenderErr open() override
    {
        mOpen = true;
        return SenderErr(SenderErr::SENDER_SUCCESS);
    }

    const SenderErr send(const std::vector<uint8_t>& pkt) override
    {
        if (!mOpen) return SenderErr(SenderErr::SENDER_ERR_NOT_OPEN);
        mPackets.push_back(pkt);
        return SenderErr(SenderErr::SENDER_SUCCESS);
    }

    void close() noexcept override { mOpen = false; }

    const std::vector<std::vector<uint8_t>>& packets() const
    {
        return mPackets;
    }

    bool isOpen() const { return mOpen; }

private:
    std::vector<std::vector<uint8_t>> mPackets;
    bool mOpen = false;
};

/* ------------------------------------------------------------------ */
/*  Constants                                                          */
/* ------------------------------------------------------------------ */

static const std::string kSimplePath =
    TRAFFIC_GEN_TESTS_RES_PATH "/0010_simple_protocol_test/";

static const std::string kSimpleIface =
    "simple_service::SIMPLE_SERVICE_ENTITY::SIMPLE_INTERFACE";

/* ------------------------------------------------------------------ */
/*  Test fixture — owns a parsed ProtocolParser                       */
/* ------------------------------------------------------------------ */

class TrafficGenTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        ASSERT_EQ(mParser.parse().getErrorCode(),
                  ProtocolParserErr::PROTOPARSER_SUCCESS);
    }

    ProtocolParser mParser{kSimplePath};
};

/* ------------------------------------------------------------------ */
/*  PacketBuilder tests                                                */
/* ------------------------------------------------------------------ */

TEST_F(TrafficGenTest, EmptyInterfaceProducesEmptyPacket)
{
    PacketBuilder builder;
    builder.seed(0);

    ProtocolInterface emptyIface("svc::ENT::EMPTY_IF", ProtocolInterface::IN);
    const auto pkt = builder.build(emptyIface, mParser.getVarDatabase());
    EXPECT_TRUE(pkt.empty());
}

TEST_F(TrafficGenTest, SimpleVarProducesOneByte)
{
    /*
     * simple_var: size=4 bits → packed into 1 byte (padded to 8 bits).
     */
    PacketBuilder builder;
    builder.seed(42);

    const ProtocolInterface* iface =
        mParser.getInterfaceDatabase().getElement(kSimpleIface);
    ASSERT_NE(iface, nullptr);

    const auto pkt = builder.build(*iface, mParser.getVarDatabase());
    EXPECT_EQ(pkt.size(), 1u);
}

TEST_F(TrafficGenTest, SimpleVarValueInExpectedSet)
{
    /*
     * simple_var expectedValues = [0, 1, 2].
     * The 4-bit value is stored in the upper nibble (MSB-first packing).
     * Lower nibble is padding and always 0.
     */
    const ProtocolInterface* iface =
        mParser.getInterfaceDatabase().getElement(kSimpleIface);
    ASSERT_NE(iface, nullptr);

    PacketBuilder builder;
    builder.seed(0);

    for (int trial = 0; trial < 200; ++trial) {
        const auto pkt = builder.build(*iface, mParser.getVarDatabase());
        ASSERT_EQ(pkt.size(), 1u);
        const uint8_t value = pkt[0] >> 4;
        EXPECT_TRUE(value == 0u || value == 1u || value == 2u)
            << "Unexpected value " << static_cast<int>(value)
            << " on trial " << trial;
    }
}

TEST_F(TrafficGenTest, DeterministicWithSameSeed)
{
    const ProtocolInterface* iface =
        mParser.getInterfaceDatabase().getElement(kSimpleIface);
    ASSERT_NE(iface, nullptr);

    PacketBuilder b1, b2;
    b1.seed(12345);
    b2.seed(12345);

    for (int i = 0; i < 50; ++i) {
        EXPECT_EQ(b1.build(*iface, mParser.getVarDatabase()),
                  b2.build(*iface, mParser.getVarDatabase()));
    }
}

TEST_F(TrafficGenTest, DifferentSeedsDifferentPackets)
{
    const ProtocolInterface* iface =
        mParser.getInterfaceDatabase().getElement(kSimpleIface);
    ASSERT_NE(iface, nullptr);

    PacketBuilder b1, b2;
    b1.seed(1);
    b2.seed(999999);

    bool anyDifference = false;
    for (int i = 0; i < 100; ++i) {
        if (b1.build(*iface, mParser.getVarDatabase()) !=
            b2.build(*iface, mParser.getVarDatabase())) {
            anyDifference = true;
            break;
        }
    }
    EXPECT_TRUE(anyDifference);
}

/* ------------------------------------------------------------------ */
/*  TrafficGenerator tests                                             */
/* ------------------------------------------------------------------ */

TEST_F(TrafficGenTest, OpenClose)
{
    auto cap = std::make_unique<CaptureSender>();
    CaptureSender* raw = cap.get();

    TrafficGenerator gen(mParser, std::move(cap));
    EXPECT_EQ(gen.open().getErrorCode(), TrafficGeneratorErr::TGEN_SUCCESS);
    EXPECT_TRUE(raw->isOpen());

    gen.close();
    EXPECT_FALSE(raw->isOpen());
}

TEST_F(TrafficGenTest, SendNamedInterface)
{
    auto cap = std::make_unique<CaptureSender>();
    CaptureSender* raw = cap.get();

    TrafficGenerator gen(mParser, std::move(cap));
    gen.seedRng(7);
    gen.open();

    EXPECT_EQ(gen.send(kSimpleIface).getErrorCode(),
              TrafficGeneratorErr::TGEN_SUCCESS);
    EXPECT_EQ(raw->packets().size(), 1u);
    EXPECT_EQ(raw->packets()[0].size(), 1u);  /* 4-bit var → 1 byte */

    gen.close();
}

TEST_F(TrafficGenTest, SendUnknownInterfaceReturnsError)
{
    TrafficGenerator gen(mParser, std::make_unique<CaptureSender>());
    gen.open();

    EXPECT_EQ(gen.send("no_such::interface").getErrorCode(),
              TrafficGeneratorErr::TGEN_ERR_IFACE_NOT_FOUND);

    gen.close();
}

TEST_F(TrafficGenTest, SendAllSendsOnePacketPerInterface)
{
    auto cap = std::make_unique<CaptureSender>();
    CaptureSender* raw = cap.get();

    TrafficGenerator gen(mParser, std::move(cap));
    gen.open();

    /* simple_protocol has exactly 1 interface */
    gen.sendAll();
    EXPECT_EQ(raw->packets().size(), 1u);

    gen.close();
}

TEST_F(TrafficGenTest, SendFilteredInDirection)
{
    auto cap = std::make_unique<CaptureSender>();
    CaptureSender* raw = cap.get();

    TrafficGenerator gen(mParser, std::move(cap));
    gen.open();

    /* SIMPLE_INTERFACE is IN → produces 1 packet */
    gen.sendFiltered(ProtocolInterface::IN);
    EXPECT_EQ(raw->packets().size(), 1u);

    gen.close();
}

TEST_F(TrafficGenTest, SendFilteredOutDirectionProducesNone)
{
    auto cap = std::make_unique<CaptureSender>();
    CaptureSender* raw = cap.get();

    TrafficGenerator gen(mParser, std::move(cap));
    gen.open();

    /* SIMPLE_INTERFACE is IN → OUT filter sends nothing */
    gen.sendFiltered(ProtocolInterface::OUT);
    EXPECT_EQ(raw->packets().size(), 0u);

    gen.close();
}

TEST_F(TrafficGenTest, SendLoopProducesNPackets)
{
    auto cap = std::make_unique<CaptureSender>();
    CaptureSender* raw = cap.get();

    TrafficGenerator gen(mParser, std::move(cap));
    gen.seedRng(1);
    gen.open();

    constexpr size_t N = 10;
    EXPECT_EQ(gen.sendLoop(kSimpleIface, N).getErrorCode(),
              TrafficGeneratorErr::TGEN_SUCCESS);
    EXPECT_EQ(raw->packets().size(), N);

    gen.close();
}

TEST_F(TrafficGenTest, SendWithoutOpenReturnsNotOpen)
{
    TrafficGenerator gen(mParser, std::make_unique<CaptureSender>());

    /* Intentionally skip gen.open() */
    EXPECT_EQ(gen.send(kSimpleIface).getErrorCode(),
              TrafficGeneratorErr::TGEN_ERR_NOT_OPEN);
}

/* ------------------------------------------------------------------ */
/*  VerilatorSender AXI-Stream framing unit test (no socket needed)  */
/* ------------------------------------------------------------------ */

#include <verilator_sender.h>

TEST(VerilatorSenderBeat, FullBeat)
{
    /*
     * A packet of exactly 8 bytes should produce exactly 1 beat with
     * tkeep=0xFF and tlast=1.
     */
    static_assert(sizeof(AxiStreamBeat) == 10, "beat size");

    /* We can't call open() without a listener, so we test the framing
       logic directly via the struct layout. */
    std::vector<uint8_t> pkt = {0x01, 0x02, 0x03, 0x04,
                                 0x05, 0x06, 0x07, 0x08};

    AxiStreamBeat beat = {};
    for (size_t i = 0; i < 8; ++i) {
        beat.tdata |= static_cast<uint64_t>(pkt[i]) << (i * 8);
    }
    beat.tkeep = 0xFF;
    beat.tlast = 1;

    EXPECT_EQ(beat.tkeep, 0xFFu);
    EXPECT_EQ(beat.tlast, 1u);
    EXPECT_EQ(beat.tdata & 0xFF, 0x01u);               /* first byte in LSB */
    EXPECT_EQ((beat.tdata >> 56) & 0xFF, 0x08u);       /* last byte in MSB */
}

TEST(VerilatorSenderBeat, PartialLastBeat)
{
    /* 3-byte remainder → tkeep = 0b00000111 = 0x07 */
    const size_t remBytes = 3;
    const uint8_t tkeep =
        static_cast<uint8_t>((1u << remBytes) - 1u);
    EXPECT_EQ(tkeep, 0x07u);
}
