/*
 * SPDX-FileCopyrightText: 2025 Kebag-Logic (https://kebag-logic.com)
 * SPDX-FileCopyrightText: 2025 Alexandre Malki <alexandre.malki@kebag-logic.com>
 * SPDX-License-Identifier: MIT
 */


 #include <ethernet_frame_logic.h>
#include <layer_context.h>
#include <logic_module.h>
#include <logic_registry.h>
#include <protocol_parser.h>
#include <stack.h>
#include <stack_builder.h>

#include <gtest/gtest.h>

#include <memory>
#include <string>

namespace {

const std::string kProtocolsDir =
    std::string(LOGIC_TESTS_RES_PATH) + "/ethernet_logic_test/protocols/";

const std::string kStackEthernetOnly =
    std::string(LOGIC_TESTS_RES_PATH)
    + "/ethernet_logic_test/stacks/ethernet_only.yaml";

const std::string kStackEthernetBypassed =
    std::string(LOGIC_TESTS_RES_PATH)
    + "/ethernet_logic_test/stacks/ethernet_bypassed.yaml";

} /* anonymous namespace */

TEST(EthernetFrameLogic, RegisteredUnderExpectedName)
{
    EXPECT_TRUE(LogicRegistry::instance().has("ethernet_mac_frame"));
    auto mod = LogicRegistry::instance().create("ethernet_mac_frame");
    ASSERT_NE(mod, nullptr);
    EXPECT_NE(dynamic_cast<EthernetFrameLogic*>(mod.get()), nullptr);
}

TEST(EthernetFrameLogic, BoundByStackBuilder)
{
    ProtocolParser parser(kProtocolsDir);
    parser.parse();

    StackBuilder builder(parser.getServiceDatabase());
    std::unique_ptr<Stack> stack;
    ASSERT_EQ(builder.build(kStackEthernetOnly, stack).getErrorCode(),
              StackBuilderErr::STACK_SUCCESS);
    ASSERT_EQ(stack->size(), 1u);

    auto* eth = dynamic_cast<EthernetFrameLogic*>(
                    stack->layers()[0]->logic.get());
    ASSERT_NE(eth, nullptr);
    EXPECT_EQ(stack->layers()[0]->logicName, "ethernet_mac_frame");
    EXPECT_FALSE(stack->layers()[0]->bypassLogic);
}

TEST(EthernetFrameLogic, BypassLogicSwapsInPassthrough)
{
    ProtocolParser parser(kProtocolsDir);
    parser.parse();

    StackBuilder builder(parser.getServiceDatabase());
    std::unique_ptr<Stack> stack;
    ASSERT_EQ(builder.build(kStackEthernetBypassed, stack).getErrorCode(),
              StackBuilderErr::STACK_SUCCESS);
    ASSERT_EQ(stack->size(), 1u);

    EXPECT_TRUE(stack->layers()[0]->bypassLogic);
    EXPECT_EQ(stack->layers()[0]->logicName, "ethernet_mac_frame");
    /* Fuzzing path: even though the service declares logic, bypass
     * forces PassthroughLogic and the real module is NOT instantiated. */
    EXPECT_NE(dynamic_cast<PassthroughLogic*>(
                  stack->layers()[0]->logic.get()), nullptr);
    EXPECT_EQ(dynamic_cast<EthernetFrameLogic*>(
                  stack->layers()[0]->logic.get()), nullptr);
}

TEST(EthernetFrameLogic, EncodeDecodeCountersAdvance)
{
    EthernetFrameLogic eth;
    LayerContext ctx("ethernet_mac_frame");

    EXPECT_EQ(eth.getEncodeCalls(), 0);
    EXPECT_EQ(eth.getDecodeCalls(), 0);

    eth.onEncode(ctx);
    eth.onDecode(ctx);

    EXPECT_EQ(eth.getEncodeCalls(), 1);
    EXPECT_EQ(eth.getDecodeCalls(), 1);
}

TEST(EthernetFrameLogic, NextLayerDefaults1722)
{
    EthernetFrameLogic eth;
    LayerContext ctx("ethernet_mac_frame");
    EXPECT_EQ(eth.nextLayer(ctx), "1722_avtp_common_stream");
}
