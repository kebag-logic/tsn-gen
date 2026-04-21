/*
 * SPDX-FileCopyrightText: 2025 Kebag-Logic (https://kebag-logic.com)
 * SPDX-FileCopyrightText: 2025 Alexandre Malki <alexandre.malki@kebag-logic.com>
 * SPDX-License-Identifier: MIT
 */


 #include <parser_common_test.h>
#include <protocol_parser.h>
#include <protocol.h>
#include <service.h>
#include <var.h>
#include <protocol_interface.h>

#include <string>

TEST(ObjectSerializer, ParseSimpleProtocol)
{
    const std::string path(PARSER_TESTS_RES_PATH
            "/0010_simple_protocol_test/");
    ProtocolParser parser(path);
    ProtocolParserErr error = parser.parse();

    EXPECT_EQ(error.getErrorCode(), ProtocolParserErr::PROTOPARSER_SUCCESS);
}

TEST(ObjectSerializer, ParseSimpleProtocolVarCount)
{
    const std::string path(PARSER_TESTS_RES_PATH
            "/0010_simple_protocol_test/");
    ProtocolParser parser(path);
    parser.parse();

    EXPECT_EQ(parser.getVarDatabase().size(), 1u);
}

TEST(ObjectSerializer, ParseSimpleProtocolVarName)
{
    const std::string path(PARSER_TESTS_RES_PATH
            "/0010_simple_protocol_test/");
    ProtocolParser parser(path);
    parser.parse();

    const Var* var = parser.getVarDatabase().getElement(
                        "simple_service::simple_var");
    ASSERT_NE(var, nullptr);

    std::string name;
    EXPECT_EQ(var->getVarName(name).getErrorCode(), VarErr::VAR_SUCCESS);
    EXPECT_STREQ(name.c_str(), "simple_service::simple_var");
}

TEST(ObjectSerializer, ParseSimpleProtocolVarSize)
{
    const std::string path(PARSER_TESTS_RES_PATH
            "/0010_simple_protocol_test/");
    ProtocolParser parser(path);
    parser.parse();

    const Var* var = parser.getVarDatabase().getElement(
                        "simple_service::simple_var");
    ASSERT_NE(var, nullptr);
    EXPECT_EQ(var->getSize(), 4u);
}

TEST(ObjectSerializer, ParseSimpleProtocolVarExpectedValues)
{
    const std::string path(PARSER_TESTS_RES_PATH
            "/0010_simple_protocol_test/");
    ProtocolParser parser(path);
    parser.parse();

    const Var* var = parser.getVarDatabase().getElement(
                        "simple_service::simple_var");
    ASSERT_NE(var, nullptr);

    const std::vector<int32_t>& expected = var->getExpectedValues();
    ASSERT_EQ(expected.size(), 3u);
    EXPECT_EQ(expected[0], 0);
    EXPECT_EQ(expected[1], 1);
    EXPECT_EQ(expected[2], 2);
}

TEST(ObjectSerializer, ParseSimpleProtocolIfaceCount)
{
    const std::string path(PARSER_TESTS_RES_PATH
            "/0010_simple_protocol_test/");
    ProtocolParser parser(path);
    parser.parse();

    EXPECT_EQ(parser.getInterfaceDatabase().size(), 1u);
}

TEST(ObjectSerializer, ParseSimpleProtocolIfaceName)
{
    const std::string path(PARSER_TESTS_RES_PATH
            "/0010_simple_protocol_test/");
    ProtocolParser parser(path);
    parser.parse();

    const ProtocolInterface* iface = parser.getInterfaceDatabase().getElement(
        "simple_service::SIMPLE_SERVICE_ENTITY::SIMPLE_INTERFACE");
    ASSERT_NE(iface, nullptr);
    EXPECT_STREQ(iface->getName().c_str(),
                 "simple_service::SIMPLE_SERVICE_ENTITY::SIMPLE_INTERFACE");
}

TEST(ObjectSerializer, ParseSimpleProtocolIfaceDirection)
{
    const std::string path(PARSER_TESTS_RES_PATH
            "/0010_simple_protocol_test/");
    ProtocolParser parser(path);
    parser.parse();

    const ProtocolInterface* iface = parser.getInterfaceDatabase().getElement(
        "simple_service::SIMPLE_SERVICE_ENTITY::SIMPLE_INTERFACE");
    ASSERT_NE(iface, nullptr);
    EXPECT_EQ(iface->getDirection(), ProtocolInterface::IN);
}

TEST(ObjectSerializer, ParseSimpleProtocolIfaceVarRef)
{
    const std::string path(PARSER_TESTS_RES_PATH
            "/0010_simple_protocol_test/");
    ProtocolParser parser(path);
    parser.parse();

    const ProtocolInterface* iface = parser.getInterfaceDatabase().getElement(
        "simple_service::SIMPLE_SERVICE_ENTITY::SIMPLE_INTERFACE");
    ASSERT_NE(iface, nullptr);

    const std::vector<std::string>& refs = iface->getVarRefs();
    ASSERT_EQ(refs.size(), 1u);
    EXPECT_EQ(refs[0], "simple_service::simple_var");
}

TEST(ObjectSerializer, ParseSimpleProtocolServiceNoLogic)
{
    const std::string path(PARSER_TESTS_RES_PATH
            "/0010_simple_protocol_test/");
    ProtocolParser parser(path);
    parser.parse();

    const ProtocolService* svc =
        parser.getServiceDatabase().getElement("simple_service");
    ASSERT_NE(svc, nullptr);
    EXPECT_EQ(svc->getName(), "simple_service");
    EXPECT_FALSE(svc->hasLogic());
    EXPECT_EQ(svc->getLogicName(), "");
}

TEST(ObjectSerializer, ParseProtocolWithLogicName)
{
    const std::string path(PARSER_TESTS_RES_PATH
            "/0030_protocol_with_logic/");
    ProtocolParser parser(path);
    parser.parse();

    const ProtocolService* svc =
        parser.getServiceDatabase().getElement("protocol_with_logic");
    ASSERT_NE(svc, nullptr);
    EXPECT_TRUE(svc->hasLogic());
    EXPECT_EQ(svc->getLogicName(), "with_logic_module");
}
