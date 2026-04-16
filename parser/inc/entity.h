/*
 * SPDX-FileCopyrightText: 2025 Kebag-Logic (https://kebag-logic.com)
 * SPDX-FileCopyrightText: 2025 Alexandre Malki <alexandre.malki@kebag-logic.com>
 * SPDX-License-Identifier: MIT
 */


 #pragma once

#include <string>
#include <vector>

/**
 * @brief An Entity belongs to a Service and holds a set of interfaces.
 */
class Entity {
public:
	Entity() = default;
	explicit Entity(const std::string& name) : mName(name) {}

	const std::string& getName() const { return mName; }

	void addInterface(const std::string& qualifiedIfName)
	{
		mInterfaces.push_back(qualifiedIfName);
	}

	const std::vector<std::string>& getInterfaces() const { return mInterfaces; }

private:
	std::string mName;
	std::vector<std::string> mInterfaces;
};
