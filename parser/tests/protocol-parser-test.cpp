#include <gtest/gtest.h>
#include <protocol-parser.h>


TEST(ParserUsingTest, Version) {
	ProtocolParser testProtocol("NOTHING");

	EXPECT_STREQ(PROTOCOL_PARSER_VERSION, testProtocol.getVersion().c_str());
}

TEST(ParserUsingTest, BasePath) {
	const std::string dummypath("NOTHING");
	ProtocolParser testProtocol(dummypath);

	EXPECT_STREQ(dummypath.c_str(), testProtocol.getBasePath().c_str());
}
