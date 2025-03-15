#include <iostream>

#include <protocol-parser.h>
#include <version.h>

ProtocolParser::ProtocolParser(const std::string& baseProtoPath):
	mProtocolParserVersion(PROTOCOL_PARSER_VERSION),
	mProtocolBasePath(baseProtoPath)
{
}

const std::string& ProtocolParser::getVersion(void) const
{
	return mProtocolParserVersion;
}

const std::string& ProtocolParser::getBasePath(void) const
{
	return mProtocolBasePath;
}
