/*
 * SPDX-FileCopyrightText: 2025 Kebag-Logic (https://kebag-logic.com)
 * SPDX-FileCopyrightText: 2025 Alexandre Malki <alexandre.malki@kebag-logic.com>
 * SPDX-License-Identifier: MIT
 */


 #pragma once

#include <cstddef>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>

#include <logic_module.h>

/**
 * @brief Process-wide string -> ILogicModule factory.
 *
 *  Logic modules register themselves at static init via REGISTER_LOGIC.
 *  The StackBuilder resolves each layer's `logic:` name against this
 *  registry when wiring a stack.
 *
 *  Notes on linkage: static init in a SHARED library runs only when
 *  that library is loaded. For a STATIC library, ensure the linker
 *  does not drop the registration TU (e.g. --whole-archive on GCC/ld).
 */
class LogicRegistry {
public:
    using Factory = std::function<std::unique_ptr<ILogicModule>()>;

    static LogicRegistry& instance();

    /**
     * @brief Register a factory under @p name.
     * @return false if the name is empty, factory is null, or the name
     *         is already registered. Existing entries are never
     *         overwritten.
     */
    bool add(const std::string& name, Factory factory);

    /**
     * @brief Instantiate a module by name.
     * @return nullptr when the name is not registered.
     */
    std::unique_ptr<ILogicModule> create(const std::string& name) const;

    bool has(const std::string& name) const;
    size_t size() const;

private:
    LogicRegistry() = default;
    LogicRegistry(const LogicRegistry&) = delete;
    LogicRegistry& operator=(const LogicRegistry&) = delete;

    std::unordered_map<std::string, Factory> mFactories;
};

/**
 * @brief Register a concrete ILogicModule subclass under a YAML name.
 *        Place this at file scope in a .cpp that links into the final
 *        binary or is force-loaded.
 *
 *        REGISTER_LOGIC("ethernet_mac_frame", EthernetFrameLogic);
 */
#define REGISTER_LOGIC(NAME, CLASS)                                         \
    namespace {                                                             \
        const bool _logic_reg_##CLASS =                                     \
            ::LogicRegistry::instance().add(                                \
                (NAME), [] { return std::make_unique<CLASS>(); });          \
    }
