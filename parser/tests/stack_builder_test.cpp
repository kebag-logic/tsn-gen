/*
 * SPDX-FileCopyrightText: 2025 Kebag-Logic (https://kebag-logic.com)
 * SPDX-FileCopyrightText: 2025 Alexandre Malki <alexandre.malki@kebag-logic.com>
 * SPDX-License-Identifier: MIT
 */


 #include <parser_common_test.h>
#include <logic_module.h>
#include <logic_registry.h>
#include <protocol_parser.h>
#include <stack.h>
#include <stack_builder.h>

#include <memory>
#include <string>

namespace {

const std::string kProtocolsDir =
    std::string(PARSER_TESTS_RES_PATH) + "/0040_stack_test/protocols/";

const std::string kStackPassthrough =
    std::string(PARSER_TESTS_RES_PATH)
    + "/0040_stack_test/stacks/passthrough.yaml";

const std::string kStackWithLogic =
    std::string(PARSER_TESTS_RES_PATH)
    + "/0040_stack_test/stacks/with_logic.yaml";

const std::string kStackUnknownLogic =
    std::string(PARSER_TESTS_RES_PATH)
    + "/0040_stack_test/stacks/unknown_logic.yaml";

const std::string kStackUnknownService =
    std::string(PARSER_TESTS_RES_PATH)
    + "/0040_stack_test/stacks/unknown_service.yaml";

const std::string kStackBypassLogic =
    std::string(PARSER_TESTS_RES_PATH)
    + "/0040_stack_test/stacks/bypass_logic.yaml";

const std::string kStackBypassUnknownLogic =
    std::string(PARSER_TESTS_RES_PATH)
    + "/0040_stack_test/stacks/bypass_unknown_logic.yaml";

struct WithLogicModule : public ILogicModule {};

} /* anonymous namespace */

/* Static-init registers the module referenced by protocol_with_logic.
 * protocol_with_unknown_logic intentionally uses a name that is never
 * registered so the unknown-logic path can be exercised. */
REGISTER_LOGIC("with_logic_module", WithLogicModule);

TEST(StackBuilder, BuildsAllPassthroughStack)
{
    ProtocolParser parser(kProtocolsDir);
    parser.parse();

    StackBuilder builder(parser.getServiceDatabase());
    std::unique_ptr<Stack> stack;
    StackBuilderErr err = builder.build(kStackPassthrough, stack);

    ASSERT_EQ(err.getErrorCode(), StackBuilderErr::STACK_SUCCESS);
    ASSERT_NE(stack, nullptr);
    EXPECT_EQ(stack->getName(), "passthrough_stack");
    ASSERT_EQ(stack->size(), 2u);

    const auto& layers = stack->layers();
    for (const auto& l : layers) {
        EXPECT_EQ(l->serviceName, "simple_service");
        EXPECT_FALSE(l->bypassLogic);
        EXPECT_EQ(l->logicName, "");
        EXPECT_NE(dynamic_cast<PassthroughLogic*>(l->logic.get()), nullptr);
    }
}

TEST(StackBuilder, ResolvesRegisteredLogic)
{
    ProtocolParser parser(kProtocolsDir);
    parser.parse();

    StackBuilder builder(parser.getServiceDatabase());
    std::unique_ptr<Stack> stack;
    StackBuilderErr err = builder.build(kStackWithLogic, stack);

    ASSERT_EQ(err.getErrorCode(), StackBuilderErr::STACK_SUCCESS);
    ASSERT_NE(stack, nullptr);
    ASSERT_EQ(stack->size(), 2u);

    const auto& layers = stack->layers();

    /* Layer 0: simple_service, no logic -> Passthrough */
    EXPECT_EQ(layers[0]->logicName, "");
    EXPECT_NE(dynamic_cast<PassthroughLogic*>(layers[0]->logic.get()),
              nullptr);

    /* Layer 1: protocol_with_logic, registered module -> WithLogicModule */
    EXPECT_EQ(layers[1]->logicName, "with_logic_module");
    EXPECT_NE(dynamic_cast<WithLogicModule*>(layers[1]->logic.get()),
              nullptr);
    EXPECT_EQ(dynamic_cast<PassthroughLogic*>(layers[1]->logic.get()),
              nullptr);
}

TEST(StackBuilder, AdjacencyWiredBottomUp)
{
    ProtocolParser parser(kProtocolsDir);
    parser.parse();

    StackBuilder builder(parser.getServiceDatabase());
    std::unique_ptr<Stack> stack;
    ASSERT_EQ(builder.build(kStackWithLogic, stack).getErrorCode(),
              StackBuilderErr::STACK_SUCCESS);
    ASSERT_EQ(stack->size(), 2u);

    const auto& layers = stack->layers();
    EXPECT_EQ(layers[0]->context.lower(), nullptr);
    EXPECT_EQ(layers[0]->context.upper(), &layers[1]->context);
    EXPECT_EQ(layers[1]->context.lower(), &layers[0]->context);
    EXPECT_EQ(layers[1]->context.upper(), nullptr);
}

TEST(StackBuilder, UnknownLogicFailsBuild)
{
    ProtocolParser parser(kProtocolsDir);
    parser.parse();

    StackBuilder builder(parser.getServiceDatabase());
    std::unique_ptr<Stack> stack;
    StackBuilderErr err = builder.build(kStackUnknownLogic, stack);

    EXPECT_EQ(err.getErrorCode(), StackBuilderErr::STACK_ERR_UNKNOWN_LOGIC);
    EXPECT_EQ(stack, nullptr);
}

TEST(StackBuilder, UnknownServiceFailsBuild)
{
    ProtocolParser parser(kProtocolsDir);
    parser.parse();

    StackBuilder builder(parser.getServiceDatabase());
    std::unique_ptr<Stack> stack;
    StackBuilderErr err = builder.build(kStackUnknownService, stack);

    EXPECT_EQ(err.getErrorCode(),
              StackBuilderErr::STACK_ERR_UNKNOWN_SERVICE);
    EXPECT_EQ(stack, nullptr);
}

TEST(StackBuilder, BypassLogicForcesPassthrough)
{
    ProtocolParser parser(kProtocolsDir);
    parser.parse();

    StackBuilder builder(parser.getServiceDatabase());
    std::unique_ptr<Stack> stack;
    ASSERT_EQ(builder.build(kStackBypassLogic, stack).getErrorCode(),
              StackBuilderErr::STACK_SUCCESS);
    ASSERT_EQ(stack->size(), 2u);

    const auto& layers = stack->layers();

    /* Layer 0: no bypass, no declared logic -> Passthrough. */
    EXPECT_FALSE(layers[0]->bypassLogic);
    EXPECT_NE(dynamic_cast<PassthroughLogic*>(layers[0]->logic.get()),
              nullptr);

    /* Layer 1: service declares `with_logic_module` (registered), but
     * bypass-logic: true overrides -> Passthrough. logicName is still
     * carried through for introspection / diagnostics. */
    EXPECT_TRUE(layers[1]->bypassLogic);
    EXPECT_EQ(layers[1]->logicName, "with_logic_module");
    EXPECT_NE(dynamic_cast<PassthroughLogic*>(layers[1]->logic.get()),
              nullptr);
    EXPECT_EQ(dynamic_cast<WithLogicModule*>(layers[1]->logic.get()),
              nullptr);
}

TEST(StackBuilder, BypassLogicSkipsRegistryLookup)
{
    /* Service declares a logic name that is NEVER registered. Without
     * bypass-logic the build would fail; with bypass-logic the
     * registry is not consulted at all. This is the fuzzing path. */
    ProtocolParser parser(kProtocolsDir);
    parser.parse();

    StackBuilder builder(parser.getServiceDatabase());
    std::unique_ptr<Stack> stack;
    StackBuilderErr err = builder.build(kStackBypassUnknownLogic, stack);

    ASSERT_EQ(err.getErrorCode(), StackBuilderErr::STACK_SUCCESS);
    ASSERT_EQ(stack->size(), 1u);

    const auto& l = stack->layers()[0];
    EXPECT_TRUE(l->bypassLogic);
    EXPECT_EQ(l->logicName, "__never_registered_in_tests__");
    EXPECT_NE(dynamic_cast<PassthroughLogic*>(l->logic.get()), nullptr);
}

TEST(StackBuilder, InvalidPathFailsBuild)
{
    ProtocolParser parser(kProtocolsDir);
    parser.parse();
    StackBuilder builder(parser.getServiceDatabase());

    std::unique_ptr<Stack> stack;
    StackBuilderErr err = builder.build(
        std::string(PARSER_TESTS_RES_PATH) + "/does/not/exist.yaml", stack);

    EXPECT_EQ(err.getErrorCode(), StackBuilderErr::STACK_ERR_INVALID_FILE);
    EXPECT_EQ(stack, nullptr);
}
