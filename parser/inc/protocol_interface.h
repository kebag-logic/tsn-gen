#pragma once

#include <string>

class ProtocolInterface {
public:
	/*
	 * This is the direction of the interface, and that 
	 * is dependent from the Protocol it belongs to
	 */
	enum IfDirections {
		IN,
		OUT,
	};
	ProtocolInterface(const std::string& interfaceName);

private:
	const std::string& mInterfaceName;
	
};