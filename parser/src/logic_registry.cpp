/*
 * SPDX-FileCopyrightText: 2025 Kebag-Logic (https://kebag-logic.com)
 * SPDX-FileCopyrightText: 2025 Alexandre Malki <alexandre.malki@kebag-logic.com>
 * SPDX-License-Identifier: MIT
 */


 #include <logic_registry.h>

LogicRegistry& LogicRegistry::instance()
{
    static LogicRegistry reg;
    return reg;
}

bool LogicRegistry::add(const std::string& name, Factory factory)
{
    if (name.empty() || !factory) {
        return false;
    }
    auto [_, inserted] = mFactories.emplace(name, std::move(factory));
    return inserted;
}

std::unique_ptr<ILogicModule> LogicRegistry::create(const std::string& name) const
{
    auto it = mFactories.find(name);
    if (it == mFactories.end()) {
        return nullptr;
    }
    return it->second();
}

bool LogicRegistry::has(const std::string& name) const
{
    return mFactories.find(name) != mFactories.end();
}

size_t LogicRegistry::size() const
{
    return mFactories.size();
}
