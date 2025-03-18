#pragma once

#include <functional>
#include <string>
#include <vector>
#include <map>


#include <var.h>


class Protocol {
public:
	/**
	 * The path is usually the same as the
	 * Namespace application::17221_...:name
	 */
	Protocol(const std::string path);
	
private:
	const std::vector<std::string> mpath;
	std::string mProtocolName;

    std::map<std::string, Entity> mEntities;
	std::map<std::reference_wrapper<std::string>, Var&> mVar;
};
