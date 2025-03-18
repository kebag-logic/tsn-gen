#pragma once

#include <string>

class ProtocoInterface {
public:
	/*
	 * This is the direction of the interface, and that 
	 * is dependent from the Protocol it belongs to
	 */
	enum IfDirections {
		IN,
		OUT,
	};
	ProtocoInterface(const std::string& interfaceName);

private:
	const std::string& mInterfaceName;
	
};
