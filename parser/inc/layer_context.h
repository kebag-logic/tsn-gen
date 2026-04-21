/*
 * SPDX-FileCopyrightText: 2025 Kebag-Logic (https://kebag-logic.com)
 * SPDX-FileCopyrightText: 2025 Alexandre Malki <alexandre.malki@kebag-logic.com>
 * SPDX-License-Identifier: MIT
 */


 #pragma once

#include <cstdint>
#include <string>
#include <utility>

/**
 * @brief Runtime context handed to an ILogicModule on encode/decode.
 *
 *  v1 exposes the minimum surface needed by PassthroughLogic and by
 *  downstream phases: service name + adjacency to upper/lower layers.
 *  Var read/write accessors are virtual so the Stack runtime (Phase 3)
 *  can back them with the real serializer without touching modules.
 */
class LayerContext {
public:
    LayerContext() = default;
    explicit LayerContext(std::string serviceName)
        : mServiceName(std::move(serviceName)) {}
    virtual ~LayerContext() = default;

    const std::string& getServiceName() const { return mServiceName; }

    LayerContext* upper() const { return mUpper; }
    LayerContext* lower() const { return mLower; }
    void setUpper(LayerContext* u) { mUpper = u; }
    void setLower(LayerContext* l) { mLower = l; }

    /**
     * @brief Read a var value from this layer by its short (unqualified) name.
     * @return true on success; false if the name is unknown at this layer.
     *         Default implementation always returns false.
     */
    virtual bool getValue(const std::string& /*name*/, uint64_t& /*out*/) const
    {
        return false;
    }

    /**
     * @brief Write a var value on this layer by its short name.
     * @return true on success; false if the name is unknown or read-only.
     */
    virtual bool setValue(const std::string& /*name*/, uint64_t /*value*/)
    {
        return false;
    }

private:
    std::string mServiceName;
    LayerContext* mUpper = nullptr;
    LayerContext* mLower = nullptr;
};
