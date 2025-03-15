#pragma once

#include <string>
#include <version.h>

class ProtocolParser {
public:
	ProtocolParser(const std::string& baseProtoPath);
	/**
	 * Get rid of the default copy constructors
	 */
	ProtocolParser(const ProtocolParser &) = delete;
	ProtocolParser& operator=(const ProtocolParser &) = delete;

	/**
	 * @brief Provide the Version of the parser
	 * @return a reference on the string
	 */
	const std::string& getVersion(void) const;

	/**
	 * @brief Provide the base path (top level directory)
	 * 	Where are located all the protocols
	 * @return a reference to the base path where the
	 * 	protocol are  specified
	 */
	const std::string& getBasePath(void) const;

private:
	const std::string mProtocolParserVersion;
	const std::string mProtocolBasePath;
};
