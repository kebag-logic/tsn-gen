#include <gtest/gtest.h>
#include <protocol_parser.h>
#include <protocol.h>

#include <string>

TEST(ParserUsingTest, Version) {
	ProtocolParser testProtocol("NOTHING");

	EXPECT_STREQ(PROTOCOL_PARSER_VERSION, testProtocol.getVersion().c_str());
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