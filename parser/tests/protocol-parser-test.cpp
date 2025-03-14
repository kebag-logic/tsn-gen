#include <gtest/gtest.h>
#include <protocol-parser.h>


TEST(ParserUsingTest, Version) {
	ProtocolParser testProtocol; 

	EXPECT_STREQ(PROTOCOL_PARSER_VERSION, testProtocol.getVersion().c_str());
}


