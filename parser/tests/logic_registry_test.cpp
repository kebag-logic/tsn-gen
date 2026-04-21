/*
 * SPDX-FileCopyrightText: 2025 Kebag-Logic (https://kebag-logic.com)
 * SPDX-FileCopyrightText: 2025 Alexandre Malki <alexandre.malki@kebag-logic.com>
 * SPDX-License-Identifier: MIT
 */


 #include <parser_common_test.h>
#include <layer_context.h>
#include <logic_module.h>
#include <logic_registry.h>

#include <memory>
#include <string>

namespace {

struct CountingLogic : public ILogicModule {
    int encodeCount = 0;
    int decodeCount = 0;

    void onEncode(LayerContext&) override { ++encodeCount; }
    void onDecode(LayerContext&) override { ++decodeCount; }
    std::string nextLayer(const LayerContext&) const override
    {
        return "upper_stub";
    }
};

} /* anonymous namespace */

TEST(LogicRegistry, RegisterAndCreate)
{
    auto& reg = LogicRegistry::instance();
    const std::string name = "logic_registry_test::counting";

    EXPECT_TRUE(reg.add(name,
        [] { return std::make_unique<CountingLogic>(); }));
    EXPECT_TRUE(reg.has(name));

    std::unique_ptr<ILogicModule> mod = reg.create(name);
    ASSERT_NE(mod, nullptr);

    LayerContext ctx("stub_service");
    mod->onEncode(ctx);
    mod->onDecode(ctx);
    EXPECT_EQ(mod->nextLayer(ctx), "upper_stub");
}

TEST(LogicRegistry, UnknownNameReturnsNull)
{
    EXPECT_EQ(LogicRegistry::instance().create(
        "logic_registry_test::nonexistent"), nullptr);
}

TEST(LogicRegistry, DuplicateRegistrationRejected)
{
    auto& reg = LogicRegistry::instance();
    const std::string name = "logic_registry_test::dup";

    EXPECT_TRUE(reg.add(name,
        [] { return std::make_unique<CountingLogic>(); }));
    EXPECT_FALSE(reg.add(name,
        [] { return std::make_unique<PassthroughLogic>(); }));

    /* First registration still wins. */
    std::unique_ptr<ILogicModule> mod = reg.create(name);
    ASSERT_NE(mod, nullptr);
    EXPECT_EQ(mod->nextLayer(LayerContext{}), "upper_stub");
}

TEST(LogicRegistry, EmptyNameRejected)
{
    EXPECT_FALSE(LogicRegistry::instance().add("",
        [] { return std::make_unique<PassthroughLogic>(); }));
}

TEST(LogicRegistry, NullFactoryRejected)
{
    EXPECT_FALSE(LogicRegistry::instance().add(
        "logic_registry_test::null", LogicRegistry::Factory{}));
}

TEST(PassthroughLogic, IsNoOp)
{
    PassthroughLogic p;
    LayerContext ctx("some_service");

    /* Should not touch the context or throw. */
    p.onEncode(ctx);
    p.onDecode(ctx);
    EXPECT_EQ(p.nextLayer(ctx), "");
}

TEST(LayerContext, AdjacencyAndDefaults)
{
    LayerContext lower("lower");
    LayerContext middle("middle");
    LayerContext upper("upper");

    middle.setLower(&lower);
    middle.setUpper(&upper);

    EXPECT_EQ(middle.lower(), &lower);
    EXPECT_EQ(middle.upper(), &upper);
    EXPECT_EQ(middle.getServiceName(), "middle");

    uint64_t out = 0;
    EXPECT_FALSE(middle.getValue("anything", out));
    EXPECT_FALSE(middle.setValue("anything", 42));
}
