#pragma once

#include <string>

class TrafficGen {
public:
	TrafficGen();
	/**
	 * @brief retrieve the version of this module
	 */
	const std::string& getVersion(void) const;
private:
	const std::string mVersion;

};
