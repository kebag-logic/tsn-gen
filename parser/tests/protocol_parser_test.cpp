#include <gtest/gtest.h>
#include <protocol_parser.h>
#include <protocol.h>

#include <string>

#ifndef PARSER_TESTS_RES_PATH
#error "Undefined PARSER_TESTS_RES_PATH"
#endif

TEST(ParserUsingTest, Version) {
	ProtocolParser testProtocol("NOTHING");

	EXPECT_STREQ(PROTOCOL_PARSER_VERSION,
				 testProtocol.getVersion().c_str());
}

TEST(ParserUsingTest, BasePath) {
	const std::string dummypath("NOTHING");
	ProtocolParser testProtocol(dummypath);

	EXPECT_STREQ(dummypath.c_str(), testProtocol.getBasePath().c_str());
}

TEST(Parser, InvalidPathInt) {
	const std::string dummypath("Unknown_path");
	ProtocolParser testProtocol(dummypath);

	ProtocolParserErr error = testProtocol.parse();

	// Check valid path ?
	EXPECT_EQ(error.getErrorCode(),
		ProtocolParserErr::PROTOPARSERERR_INVALID_INPUT);
}

TEST(Parser, VarCreation)
{
	std::string varNameTest = "nameofthevar";
	std::string varToCheck = "";
	Var myVar(std::string("nameofthevar"));

	VarErr error = myVar.getVarName(varToCheck);
	EXPECT_EQ(error.getErrorCode(), VarErr::VAR_SUCCESS);

	EXPECT_STREQ(varToCheck.c_str(), varNameTest.c_str());

}

TEST(Parser, VarCreationTooSmall)
{
	std::string varNameTest = "";
	std::string varToCheck = "";
	Var myVar(varNameTest);

	VarErr error = myVar.getVarName(varToCheck);
	EXPECT_EQ(error.getErrorCode(), VarErr::VAR_ERR_UNVALID_VAR);
}

TEST(Parser, listFiles)
{
	const std::string dummypath(PARSER_TESTS_RES_PATH
			"/0001_yaml_but_not_a_protocol_description/");
	ProtocolParser protoParser(dummypath);

	ProtocolParserErr error = protoParser.parse();

	EXPECT_EQ(error.getErrorCode(),
			  ProtocolParserErr::PROTOPARSERERR_INVALID_INPUT);
}

TEST(Parser, VerifyDepth)
{
	const std::string dummypath(PARSER_TESTS_RES_PATH
			"/0002_max_depth_reached_out");
	ProtocolParser protoParser(dummypath);

	ProtocolParserErr error = protoParser.parse();

	EXPECT_EQ(error.getErrorCode(),
			  ProtocolParserErr::PROTOPARSERERR_UNEXPECTED);
}

TEST(Parser, DivergingPath)
{
	const std::string dummypath(PARSER_TESTS_RES_PATH
			"/0003_diverging_path");
	ProtocolParser protoParser(dummypath);

	ProtocolParserErr error = protoParser.parse();

	EXPECT_EQ(error.getErrorCode(),
			  ProtocolParserErr::PROTOPARSERERR_INVALID_INPUT);
}

TEST(Parser, ParsingProtocolNameInvalid)
{
	const std::string dummypath(PARSER_TESTS_RES_PATH
			"/0004_invalid_protocol_name");
	ProtocolParser protoParser(dummypath);

	ProtocolParserErr error = protoParser.parse();

	EXPECT_EQ(error.getErrorCode(),
			  ProtocolParserErr::PROTOPARSERERR_UNEXPECTED);
}
