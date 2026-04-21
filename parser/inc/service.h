/*
 * SPDX-FileCopyrightText: 2025 Kebag-Logic (https://kebag-logic.com)
 * SPDX-FileCopyrightText: 2025 Alexandre Malki <alexandre.malki@kebag-logic.com>
 * SPDX-License-Identifier: MIT
 */


 #pragma once

#include <string>
#include <utility>

/**
 * @brief Per-service metadata parsed from the top of a protocol YAML.
 *
 *  For now this is just the service name and the optional `logic:` key
 *  (resolved by the StackBuilder against LogicRegistry). Extend as
 *  more per-service metadata is promoted out of the raw YAML.
 */
class ProtocolService {
public:
    ProtocolService() = default;

    ProtocolService(std::string name, std::string logicName = {})
        : mName(std::move(name)), mLogicName(std::move(logicName)) {}

    const std::string& getName() const { return mName; }

    /** @return the module name declared by `logic:`, or "" when absent. */
    const std::string& getLogicName() const { return mLogicName; }
    bool hasLogic() const { return !mLogicName.empty(); }

private:
    std::string mName;
    std::string mLogicName;
};
