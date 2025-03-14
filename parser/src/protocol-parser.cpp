#include <iostream>

#include <protocol-parser.h>
#include <version.h>

ProtocolParser::ProtocolParser(void):
	mProtocolParserVersion(PROTOCOL_PARSER_VERSION)
{
}

const std::string& ProtocolParser::getVersion(void) const
{
	return mProtocolParserVersion;
}
