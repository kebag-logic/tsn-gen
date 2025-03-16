#include <gtest/gtest.h>
#include <protocol_parser.h>
#include <protocol_node.h>

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
	std::vector<ProtocolNode> protoNodes;
	 
	const ProtocolParserErr& error = testProtocol.parse(protoNodes);

	// Check valid path ?
	EXPECT_EQ(error.getErrorCode(),
		ProtocolParserErr::PROTOPARSERERR_INVALID_PATH);
}
