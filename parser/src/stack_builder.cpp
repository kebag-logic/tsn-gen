/*
 * SPDX-FileCopyrightText: 2025 Kebag-Logic (https://kebag-logic.com)
 * SPDX-FileCopyrightText: 2025 Alexandre Malki <alexandre.malki@kebag-logic.com>
 * SPDX-License-Identifier: MIT
 */


 #include <stack_builder.h>
#include <utils.h>

#include <cstdint>
#include <cstring>
#include <iostream>
#include <memory>
#include <string>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

#include <ryml.hpp>

namespace {

std::string csubstrToString(ryml::csubstr s)
{
    return std::string(s.str, s.len);
}

bool readBool(ryml::ConstNodeRef node)
{
    if (node.invalid() || !node.has_val()) {
        return false;
    }
    std::string v = csubstrToString(node.val());
    return v == "true" || v == "1" || v == "yes";
}

} /* anonymous namespace */

StackBuilder::StackBuilder(const Database<ProtocolService>& dbServices,
                           const LogicRegistry& registry)
    : mDbServices(dbServices), mRegistry(registry)
{
}

StackBuilderErr StackBuilder::build(const std::string& path,
                                    std::unique_ptr<Stack>& outStack)
{
    StackBuilderErr err(StackBuilderErr::STACK_ERR_UNKNOWN);
    outStack.reset();

    ScopedFD fd(open(path.c_str(), O_RDONLY));
    if (fd < 0) {
        std::cerr << "ERR: could not open stack file " << path << std::endl;
        err.setErrorCode(StackBuilderErr::STACK_ERR_INVALID_FILE);
        return err;
    }

    off_t len = lseek(fd, 0, SEEK_END);
    if (len <= 0) {
        err.setErrorCode(StackBuilderErr::STACK_ERR_INVALID_FILE);
        return err;
    }

    void* ptr = mmap(0, len, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
    if (ptr == MAP_FAILED) {
        err.setErrorCode(StackBuilderErr::STACK_ERR_INVALID_FILE);
        return err;
    }

    ryml::Tree tree = ryml::parse_in_arena(
        ryml::csubstr(path.c_str()),
        ryml::csubstr(static_cast<char*>(ptr), static_cast<size_t>(len)));

    munmap(ptr, len);

    if (tree.empty() || !tree.rootref().is_map()) {
        std::cerr << "ERR: stack file not a YAML map: " << path << std::endl;
        err.setErrorCode(StackBuilderErr::STACK_ERR_INVALID_FILE);
        return err;
    }

    ryml::ConstNodeRef nameNode = tree["stack"];
    if (nameNode.invalid() || !nameNode.has_val()) {
        std::cerr << "ERR: stack file missing 'stack:' key: "
                  << path << std::endl;
        err.setErrorCode(StackBuilderErr::STACK_ERR_MISSING_KEY);
        return err;
    }

    ryml::ConstNodeRef layersNode = tree["layers"];
    if (layersNode.invalid() || !layersNode.is_seq()) {
        std::cerr << "ERR: stack file missing 'layers:' sequence: "
                  << path << std::endl;
        err.setErrorCode(StackBuilderErr::STACK_ERR_MISSING_KEY);
        return err;
    }

    auto stack = std::make_unique<Stack>(csubstrToString(nameNode.val()));

    for (ryml::ConstNodeRef layerNode : layersNode) {
        if (!layerNode.is_map() || !layerNode.has_child("service")) {
            err.setErrorCode(StackBuilderErr::STACK_ERR_MISSING_KEY);
            return err;
        }

        auto layer = std::make_unique<StackLayer>();
        layer->serviceName = csubstrToString(layerNode["service"].val());
        if (layerNode.has_child("entity")) {
            layer->entityName = csubstrToString(layerNode["entity"].val());
        }
        if (layerNode.has_child("bypass-logic")) {
            layer->bypassLogic = readBool(layerNode["bypass-logic"]);
        }

        const ProtocolService* svc =
            mDbServices.getElement(layer->serviceName);
        if (!svc) {
            std::cerr << "ERR: unknown service '" << layer->serviceName
                      << "' referenced by stack " << path << std::endl;
            err.setErrorCode(StackBuilderErr::STACK_ERR_UNKNOWN_SERVICE);
            return err;
        }

        layer->logicName = svc->getLogicName();
        layer->context = LayerContext(layer->serviceName);

        if (layer->bypassLogic || !svc->hasLogic()) {
            layer->logic = std::make_unique<PassthroughLogic>();
        } else {
            layer->logic = mRegistry.create(svc->getLogicName());
            if (!layer->logic) {
                std::cerr << "ERR: logic module '" << svc->getLogicName()
                          << "' not registered (service "
                          << layer->serviceName << ")" << std::endl;
                err.setErrorCode(StackBuilderErr::STACK_ERR_UNKNOWN_LOGIC);
                return err;
            }
        }

        stack->addLayer(std::move(layer));
    }

    stack->wireAdjacency();
    outStack = std::move(stack);

    err.setErrorCode(StackBuilderErr::STACK_SUCCESS);
    return err;
}
