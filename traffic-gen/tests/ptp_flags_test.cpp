/*
 * SPDX-FileCopyrightText: 2026 Kebag-Logic (https://kebag-logic.com)
 * SPDX-FileCopyrightText: 2026 Alexandre Malki <alexandre.malki@kebag-logic.com>
 * SPDX-License-Identifier: MIT
 *
 * IEEE 802.1AS-2011 media-dependent TX flag oracle, over the shipped
 * protocols/ corpus (not a fixture copy, so the models these tests grade are
 * the ones a consumer loads).
 *
 * 11.4.2 inherits the common header "except as noted in the following
 * subclauses", and 11.4.2.3 is such an exception for the whole flags field:
 * only the bits Table 11-4 defines for the message type may be TRUE, and
 * "for message types where the bit is not defined in Table 11-4, the value of
 * the bit is set to FALSE". Table 11-4 defines twoStepFlag alone, for Sync
 * and Pdelay_Resp -- octet 0 bit 2 once Cor1-2013 renumbers it from bit 1,
 * i.e. 0x0200 with 6.3.4.2 numbering bits 1 (LSB) to 8 (MSB). Milan v1.2
 * 4.2.6 adopts that profile with Cor1/Cor2 and adds no flag exception, so the
 * conforming transmitted flags of the five Ethernet messages are:
 *
 *   Pdelay_Req 0x0000 | Pdelay_Resp 0x0200 | Pdelay_Resp_Follow_Up 0x0000
 *   Sync       0x0200 | Follow_Up   0x0000
 *
 * Two independent bindings of that table are asserted below -- what each
 * model PERMITS and what the builder actually EMITS on the wire -- plus the
 * property that separates a transmit oracle from receive tolerance: the
 * decoder recovers an out-of-profile flags word unchanged and never consults
 * `expected`, while the generator can never produce one.
 */

#include <gtest/gtest.h>

#include <tsn/packet_builder.h>
#include <tsn/packet_decoder.h>
#include <tsn/protocol_interface.h>
#include <tsn/protocol_parser.h>
#include <tsn/var.h>

using namespace tsn;

#include <cstdint>
#include <iterator>
#include <string>
#include <vector>

#ifndef PROTOCOLS_ROOT_DIR
#error "PROTOCOLS_ROOT_DIR not defined"
#endif

namespace {

/*
 * Wire offset of the two flags octets inside every PTP common header
 * (10.5.2 / Table 10-4): 1 B transportSpecific+messageType, 1 B
 * reserved+versionPTP, 2 B messageLength, 1 B domainNumber, 1 B reserved.
 */
constexpr size_t kFlagsOffset = 6;

/*
 * The common-header ptpTimescale position: Table 10-6 octet 1 bit 3,
 * renumbered to bit 4 by Cor1-2013, i.e. 0x0008 in the 16-bit word. Table
 * 11-4 defines no such bit for any media-dependent message, so a conforming
 * Sync/Follow_Up/Pdelay never carries it. This is precisely the bit the
 * superseded relaxed Sync (0x0200|0x0208) and Follow_Up (0x0000|0x0008)
 * models also permitted.
 */
constexpr uint64_t kPtpTimescale = 0x0008;

/* One media-dependent message and the flags 11.4.2.3 requires it to send. */
struct MediaDependentMessage {
    const char* flagsVar;      /* qualified var name in the var database */
    const char* iface;         /* qualified interface name              */
    uint64_t    conformingFlags;
    size_t      frameBytes;    /* messageLength, == the built frame size */
};

const MediaDependentMessage kMessages[] = {
    {"as_sync::flags",
     "as_sync::AS_SYNC::AS_SYNC_IF", 0x0200, 44},
    {"as_follow_up::flags",
     "as_follow_up::AS_FOLLOW_UP::AS_FOLLOW_UP_IF", 0x0000, 76},
    {"as_pdelay_req::flags",
     "as_pdelay_req::AS_PDELAY_REQ::AS_PDELAY_REQ_IF", 0x0000, 54},
    {"as_pdelay_resp::flags",
     "as_pdelay_resp::AS_PDELAY_RESP::AS_PDELAY_RESP_IF", 0x0200, 54},
    {"as_pdelay_resp_fu::flags",
     "as_pdelay_resp_fu::AS_PDELAY_RESP_FU::AS_PDELAY_RESP_FU_IF", 0x0000, 54},
};

const MediaDependentMessage& kSync = kMessages[0];
const MediaDependentMessage& kFollowUp = kMessages[1];
constexpr size_t kFirstPdelay = 2;   /* index of Pdelay_Req in kMessages */

/* Enough draws that a two-element `expected` list would have to be hit. */
constexpr int kTrials = 200;

/* Read the 16-bit flags word out of a built frame, MSB octet first. */
uint64_t wireFlags(const std::vector<uint8_t>& frame)
{
    return (static_cast<uint64_t>(frame[kFlagsOffset]) << 8) |
           frame[kFlagsOffset + 1];
}

/*
 * Overwrite the flags octets of a frame. The receive-tolerance tests below
 * put the word under test on the wire themselves instead of taking whatever
 * the model made the builder pick, so they keep asserting decoder behaviour
 * -- and nothing else -- however the transmit models are edited.
 */
void setWireFlags(std::vector<uint8_t>& frame, uint64_t flags)
{
    frame[kFlagsOffset] = static_cast<uint8_t>((flags >> 8) & 0xFF);
    frame[kFlagsOffset + 1] = static_cast<uint8_t>(flags & 0xFF);
}

} /* namespace */

class PtpFlags : public ::testing::Test {
protected:
    void SetUp() override
    {
        ASSERT_EQ(mParser.parse().getErrorCode(),
                  ProtocolParserErr::PROTOPARSER_SUCCESS);
    }

    const Var* flagsVar(const std::string& qualified) const
    {
        return mParser.getVarDatabase().getElement(qualified);
    }

    const ProtocolInterface* iface(const std::string& qualified) const
    {
        return mParser.getInterfaceDatabase().getElement(qualified);
    }

    /* Build one frame for a message, seeded per trial for reproducibility. */
    std::vector<uint8_t> buildFrame(const MediaDependentMessage& msg,
                                     uint64_t seed)
    {
        const ProtocolInterface* i = iface(msg.iface);
        if (i == nullptr) return {};
        PacketBuilder builder;
        builder.seed(seed);
        return builder.build(*i, mParser.getVarDatabase());
    }

    ProtocolParser mParser{PROTOCOLS_ROOT_DIR};
};

/* ------------------------------------------------------------------ */
/*  1. Model contract: what each `expected` set permits                */
/* ------------------------------------------------------------------ */

TEST_F(PtpFlags, SyncModelPermitsOnlyTwoStepFlag)
{
    const Var* v = flagsVar("as_sync::flags");
    ASSERT_NE(v, nullptr);
    EXPECT_EQ(v->getSize(), 16u);
    ASSERT_EQ(v->getExpectedValues().size(), 1u)
        << "Table 11-4 leaves Sync exactly one legal flags word";
    EXPECT_EQ(v->getExpectedValues()[0], uint64_t{0x0200});
    /* A range or mask would reopen the field behind the pinned value. */
    EXPECT_FALSE(v->hasRange());
    EXPECT_FALSE(v->hasMask());
}

TEST_F(PtpFlags, FollowUpModelPermitsOnlyAllFlagsClear)
{
    const Var* v = flagsVar("as_follow_up::flags");
    ASSERT_NE(v, nullptr);
    EXPECT_EQ(v->getSize(), 16u);
    ASSERT_EQ(v->getExpectedValues().size(), 1u)
        << "Table 11-4 defines no flag bit for Follow_Up";
    EXPECT_EQ(v->getExpectedValues()[0], uint64_t{0x0000});
    EXPECT_FALSE(v->hasRange());
    EXPECT_FALSE(v->hasMask());
}

TEST_F(PtpFlags, PdelayModelsKeepTheirStrictFlags)
{
    const struct { const char* var; uint64_t value; } kPdelay[] = {
        {"as_pdelay_req::flags",     0x0000},
        {"as_pdelay_resp::flags",    0x0200},
        {"as_pdelay_resp_fu::flags", 0x0000},
    };

    for (const auto& p : kPdelay) {
        const Var* v = flagsVar(p.var);
        ASSERT_NE(v, nullptr) << p.var;
        ASSERT_EQ(v->getExpectedValues().size(), 1u) << p.var;
        EXPECT_EQ(v->getExpectedValues()[0], p.value) << p.var;
    }
}

/*
 * The standing gate against reintroducing either superseded alternative:
 * ptpTimescale is not defined for any media-dependent message, so no model
 * may offer a value carrying it, whatever form the constraint takes.
 */
TEST_F(PtpFlags, NoMediaDependentModelPermitsPtpTimescale)
{
    for (const auto& msg : kMessages) {
        const Var* v = flagsVar(msg.flagsVar);
        ASSERT_NE(v, nullptr) << msg.flagsVar;
        ASSERT_FALSE(v->getExpectedValues().empty())
            << msg.flagsVar << " must stay constrained";

        for (const uint64_t permitted : v->getExpectedValues()) {
            EXPECT_EQ(permitted & kPtpTimescale, uint64_t{0})
                << msg.flagsVar << " permits 0x" << std::hex << permitted
                << ", which sets the common-header ptpTimescale bit that "
                   "11.4.2.3 / Table 11-4 leave FALSE for this message";
        }
        EXPECT_EQ(v->getExpectedValues().size(), 1u)
            << msg.flagsVar << " has more than one permitted flags word";
    }
}

/* ------------------------------------------------------------------ */
/*  2. Generated wire bytes: what the builder actually emits           */
/* ------------------------------------------------------------------ */

TEST_F(PtpFlags, GeneratedSyncFramesCarryTwoStepFlagOnly)
{
    for (int trial = 0; trial < kTrials; ++trial) {
        const auto frame = buildFrame(kSync, static_cast<uint64_t>(trial));
        ASSERT_EQ(frame.size(), kSync.frameBytes) << "trial " << trial;
        EXPECT_EQ(wireFlags(frame), uint64_t{0x0200})
            << "trial " << trial << ": octets " << std::hex
            << unsigned{frame[kFlagsOffset]} << " "
            << unsigned{frame[kFlagsOffset + 1]};
    }
}

TEST_F(PtpFlags, GeneratedFollowUpFramesCarryNoFlags)
{
    for (int trial = 0; trial < kTrials; ++trial) {
        const auto frame = buildFrame(kFollowUp, static_cast<uint64_t>(trial));
        ASSERT_EQ(frame.size(), kFollowUp.frameBytes) << "trial " << trial;
        EXPECT_EQ(wireFlags(frame), uint64_t{0x0000})
            << "trial " << trial << ": octets " << std::hex
            << unsigned{frame[kFlagsOffset]} << " "
            << unsigned{frame[kFlagsOffset + 1]};
    }
}

TEST_F(PtpFlags, GeneratedPdelayFramesKeepTheirStrictFlags)
{
    for (size_t m = kFirstPdelay; m < std::size(kMessages); ++m) {
        const MediaDependentMessage& msg = kMessages[m];
        for (int trial = 0; trial < kTrials; ++trial) {
            const auto frame = buildFrame(msg, static_cast<uint64_t>(trial));
            ASSERT_EQ(frame.size(), msg.frameBytes)
                << msg.iface << " trial " << trial;
            EXPECT_EQ(wireFlags(frame), msg.conformingFlags)
                << msg.iface << " trial " << trial;
        }
    }
}

/*
 * Every generated media-dependent frame agrees with the value the model
 * permits, on the wire and in the reported field list. Same draws as above,
 * stated once over the five messages the kMessages table lists explicitly.
 * That table is not discovered from the corpus: a media-dependent model
 * added later must be added to kMessages to be graded here.
 */
TEST_F(PtpFlags, GeneratedFramesMatchTheirModelsFlags)
{
    for (const auto& msg : kMessages) {
        const ProtocolInterface* i = iface(msg.iface);
        ASSERT_NE(i, nullptr) << msg.iface;

        PacketBuilder builder;
        builder.seed(0xA51D);
        for (int trial = 0; trial < kTrials; ++trial) {
            const auto pkt =
                builder.buildWithFields(*i, mParser.getVarDatabase());
            ASSERT_EQ(pkt.bytes.size(), msg.frameBytes)
                << msg.iface << " trial " << trial;

            uint64_t reported = ~uint64_t{0};
            for (const auto& f : pkt.fields) {
                if (f.first == "flags") reported = f.second;
            }
            EXPECT_EQ(reported, msg.conformingFlags)
                << msg.iface << " trial " << trial;
            EXPECT_EQ(wireFlags(pkt.bytes), msg.conformingFlags)
                << msg.iface << " trial " << trial;
        }
    }
}

/* ------------------------------------------------------------------ */
/*  3. Generation versus receive tolerance                             */
/* ------------------------------------------------------------------ */

/*
 * 11.4.1 makes reserved bits ignored on reception, and Table 11-4 marks
 * twoStepFlag itself ignored on reception. tsn-gen's decoder is compatible
 * with that by construction, because it never applies `expected`: it recovers
 * whatever is on the wire. Asserted here on the exact out-of-profile word the
 * relaxed models used to permit, so tightening generation is shown not to
 * narrow what can be decoded.
 *
 * This is a statement about this decoder only. It is not a claim that any
 * receiving DUT admits the frame: admission depends on framing, identity,
 * pairing and state, which live in the consuming harness.
 */
TEST_F(PtpFlags, DecoderRecoversOutOfProfileFlagsVerbatim)
{
    const ProtocolInterface* i = iface(kSync.iface);
    ASSERT_NE(i, nullptr);

    auto conforming = buildFrame(kSync, 7);
    ASSERT_EQ(conforming.size(), kSync.frameBytes);
    setWireFlags(conforming, kSync.conformingFlags);

    /* The one-bit difference between a conforming Sync and the 0x0208 one. */
    auto outOfProfile = conforming;
    outOfProfile[kFlagsOffset + 1] |= static_cast<uint8_t>(kPtpTimescale);
    ASSERT_EQ(wireFlags(outOfProfile), uint64_t{0x0208});

    PacketDecoder decoder;
    const auto a = decoder.decode(*i, mParser.getVarDatabase(), conforming);
    const auto b = decoder.decode(*i, mParser.getVarDatabase(), outOfProfile);

    /* Decoded, not rejected, and returned byte-for-byte unnormalised. */
    EXPECT_EQ(b.bytes, outOfProfile);
    ASSERT_EQ(a.fields.size(), b.fields.size());
    for (size_t f = 0; f < a.fields.size(); ++f) {
        ASSERT_EQ(a.fields[f].first, b.fields[f].first);
        if (a.fields[f].first == "flags") {
            EXPECT_EQ(a.fields[f].second, uint64_t{0x0200});
            EXPECT_EQ(b.fields[f].second, uint64_t{0x0208});
        } else {
            EXPECT_EQ(a.fields[f].second, b.fields[f].second)
                << "field '" << a.fields[f].first
                << "' changed although only the reserved flag bit differs";
        }
    }
}

/*
 * The two directions are independent, and this is the assertion that says so:
 * the decoder hands back a value the transmit model forbids. Grading a decode
 * therefore requires the consuming harness to compare the recovered field
 * against the model -- the decoder alone is not a conformance filter.
 */
TEST_F(PtpFlags, DecodingDoesNotApplyTheTransmitConstraint)
{
    const ProtocolInterface* i = iface(kFollowUp.iface);
    const Var* v = flagsVar(kFollowUp.flagsVar);
    ASSERT_NE(i, nullptr);
    ASSERT_NE(v, nullptr);

    auto frame = buildFrame(kFollowUp, 11);
    ASSERT_EQ(frame.size(), kFollowUp.frameBytes);
    setWireFlags(frame, kFollowUp.conformingFlags | kPtpTimescale);

    PacketDecoder decoder;
    const auto decoded = decoder.decode(*i, mParser.getVarDatabase(), frame);

    uint64_t recovered = ~uint64_t{0};
    for (const auto& f : decoded.fields) {
        if (f.first == "flags") recovered = f.second;
    }
    EXPECT_EQ(recovered, uint64_t{0x0008});

    bool permittedForTransmission = false;
    for (const uint64_t permitted : v->getExpectedValues()) {
        if (permitted == recovered) permittedForTransmission = true;
    }
    EXPECT_FALSE(permittedForTransmission)
        << "0x0008 must stay decodable while remaining a nonconforming "
           "Follow_Up transmission";
}
