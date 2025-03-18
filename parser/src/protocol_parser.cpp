#include <iostream>
#include <filesystem>

#include <protocol_parser.h>
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

const ProtocolParserErr ProtocolParser::parse(void)
{
	ProtocolParserErr err;

	err.setErrorCode(ProtocolParserErr::PROTOPARSERERR_INVALID_INPUT);
	return err;
}

// TODO create its own class
const ProtocolParserErr ProtocolParser::parseInterfaces(void)
{
	ProtocolParserErr err;

	err.setErrorCode(ProtocolParserErr::PROTOPARSERERR_INVALID_INPUT);
	return err;
}

// TODO create its own classfg
const ProtocolParserErr ProtocolParser::parseVars(void)
{
	ProtocolParserErr err;
	

	return err;
}
