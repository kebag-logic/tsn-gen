/*
 * SPDX-FileCopyrightText: 2025 Kebag-Logic (https://kebag-logic.com)
 * SPDX-FileCopyrightText: 2025 Alexandre Malki <alexandre.malki@kebag-logic.com>
 * SPDX-License-Identifier: MIT
 */


 #pragma once

#include <memory>
#include <string>
#include <vector>

#include <database.h>
#include <error.h>
#include <logic_registry.h>
#include <service.h>
#include <stack.h>

class StackBuilderErr : public Error {
public:
    enum {
        STACK_SUCCESS,
        STACK_ERR_UNKNOWN,
        STACK_ERR_INVALID_FILE,
        STACK_ERR_MISSING_KEY,
        STACK_ERR_UNKNOWN_SERVICE,
        STACK_ERR_UNKNOWN_LOGIC,

        STACK_ERR_ENUM_SENTINEL
    };

    StackBuilderErr() = delete;
    StackBuilderErr(int errorNum) : Error(errorNum, mErrVector) {}

    StackBuilderErr& operator=(const StackBuilderErr other)
    {
        mErrorCode = other.mErrorCode;
        return *this;
    }

private:
    const std::vector<std::string> mErrVector = {
        "STACK:ERR: SUCCESS",
        "STACK:ERR: UNKNOWN",
        "STACK:ERR: INVALID FILE",
        "STACK:ERR: MISSING KEY",
        "STACK:ERR: UNKNOWN SERVICE",
        "STACK:ERR: UNKNOWN LOGIC",
    };
};

/**
 * @brief Parses a stack YAML and produces a bound Stack.
 *
 *  Expected stack YAML shape:
 *
 *      stack: <name>
 *      layers:
 *        - service: <service_name>
 *          entity:  <entity_name>
 *        - service: <service_name>
 *          entity:  <entity_name>
 *          bypass-logic: true          # optional (Phase 4)
 *
 *  Resolution rules per layer:
 *      - Service must exist in the provided service database.
 *      - If bypass-logic: true          -> PassthroughLogic.
 *      - Else if service declared logic:-> registry.create(name).
 *          (hard error when name is not registered)
 *      - Else                           -> PassthroughLogic.
 */
class StackBuilder {
public:
    StackBuilder(const Database<ProtocolService>& dbServices,
                 const LogicRegistry& registry = LogicRegistry::instance());

    /**
     * @brief Build a Stack from the YAML at @p path.
     * @param [out] outStack on success, holds the built Stack. Left empty
     *              (nullptr) on failure.
     */
    StackBuilderErr build(const std::string& path,
                          std::unique_ptr<Stack>& outStack);

private:
    const Database<ProtocolService>& mDbServices;
    const LogicRegistry& mRegistry;
};
