/*
 * SPDX-FileCopyrightText: 2025 Kebag-Logic (https://kebag-logic.com)
 * SPDX-FileCopyrightText: 2025 Alexandre Malki <alexandre.malki@kebag-logic.com>
 * SPDX-License-Identifier: MIT
 */


 #pragma once

#include <cstddef>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <layer_context.h>
#include <logic_module.h>

/**
 * @brief One layer of a built Stack: a resolved service + entity, the
 *        logic module bound to it (always non-null — PassthroughLogic
 *        if the YAML declared none or bypass-logic was set), and a
 *        runtime context.
 *
 *  Stored as unique_ptr inside Stack so that LayerContext addresses
 *  remain stable across growth / moves — the adjacency pointers set
 *  up by wireAdjacency() rely on that.
 */
struct StackLayer {
    std::string serviceName;
    std::string entityName;

    /** Name declared by `logic:` in the service YAML, or "" if none. */
    std::string logicName;

    /** True when `bypass-logic: true` on this layer forced Passthrough. */
    bool bypassLogic = false;

    std::unique_ptr<ILogicModule> logic;
    LayerContext context;
};

/**
 * @brief An ordered chain of bound layers, built by StackBuilder.
 *
 *  Order is bottom-up: index 0 is the lowest layer (e.g. Ethernet MAC),
 *  the last entry is the highest (application payload). Encode / decode
 *  walkers (added alongside the serializer) traverse in that order.
 */
class Stack {
public:
    using LayerPtr = std::unique_ptr<StackLayer>;

    explicit Stack(std::string name) : mName(std::move(name)) {}

    const std::string& getName() const { return mName; }
    size_t size() const { return mLayers.size(); }
    const std::vector<LayerPtr>& layers() const { return mLayers; }

    void addLayer(LayerPtr layer) { mLayers.push_back(std::move(layer)); }

    /**
     * @brief Populate each LayerContext's upper/lower pointers after
     *        all layers have been added. Call once, post-build.
     */
    void wireAdjacency();

private:
    std::string mName;
    std::vector<LayerPtr> mLayers;
};
