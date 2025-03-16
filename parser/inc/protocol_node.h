#pragma once

#include <string>

class ProtocolNode {
public:
	ProtocolNode(const std::string& nodePath);

private:
	const std::string mPath;
};
