#pragma once

#include <string>
#include <version.h>

class ProtocolParser {
public:
	ProtocolParser();
	~ProtocolParser() = default;

	/**
	 * @brief Provide the Version of the parser
	 * @return a reference on the string
	 */
	const std::string& getVersion(void) const;

private:
	const std::string mProtocolParserVersion;
};
