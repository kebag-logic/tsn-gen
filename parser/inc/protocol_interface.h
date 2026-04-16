/*
 * SPDX-FileCopyrightText: 2025 Kebag-Logic (https://kebag-logic.com)
 * SPDX-FileCopyrightText: 2025 Alexandre Malki <alexandre.malki@kebag-logic.com>
 * SPDX-License-Identifier: MIT
 */


 #pragma once

#include <string>
#include <vector>

class ProtocolInterface {
public:
	/*
	 * Direction of the interface from the perspective of the protocol entity.
	 */
	enum IfDirections {
		IN,
		OUT,
	};

	ProtocolInterface(const std::string& interfaceName, IfDirections dir,
	                  std::vector<std::string> varRefs = {})
	    : mInterfaceName(interfaceName),
	      mDirection(dir),
	      mVarRefs(std::move(varRefs)) {}

	ProtocolInterface() = delete;

	const std::string& getName() const { return mInterfaceName; }
	IfDirections getDirection() const { return mDirection; }
	const std::vector<std::string>& getVarRefs() const { return mVarRefs; }

private:
	const std::string mInterfaceName;
	const IfDirections mDirection;
	const std::vector<std::string> mVarRefs;
};
