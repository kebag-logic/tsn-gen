/*
 * SPDX-FileCopyrightText: 2026 Kebag-Logic (https://kebag-logic.com)
 * SPDX-FileCopyrightText: 2026 Alexandre Malki <alexandre.malki@kebag-logic.com>
 * SPDX-License-Identifier: MIT
 */

#include <tsn/session.h>

#include <cstdlib>
#include <unordered_map>

#include <tsn/log.h>

namespace tsn {

Session::Session(std::string protocolDir)
    : mProtocolDir(std::move(protocolDir)),
      mParser(mProtocolDir),
      mBuilder()
{
}

bool Session::parse()
{
    const ProtocolParserErr err = mParser.parse();
    if (err.getErrorCode() != ProtocolParserErr::PROTOPARSER_SUCCESS) {
        log(LogLevel::error) << "Session: parsing '" << mProtocolDir
                             << "' failed: " << err.getErrorMsg() << "\n";
        return false;
    }
    return true;
}

const ProtocolInterface* Session::findInterface(const std::string& name) const
{
    return mParser.getInterfaceDatabase().getElement(name);
}

PacketBuilder::BuiltPacket Session::generate(const ProtocolInterface& iface)
{
    return mBuilder.buildWithFields(iface, mParser.getVarDatabase());
}

std::vector<Session::Layer> Session::generateChain(
    const std::vector<const ProtocolInterface*>& stages)
{
    std::vector<Layer> round;
    round.reserve(stages.size());

    std::unordered_map<std::string, uint64_t> overrides;

    for (size_t stage = 0; stage < stages.size(); ++stage) {
        const ProtocolInterface& iface = *stages[stage];

        PacketBuilder::BuiltPacket pkt =
            (stage == 0)
            ? mBuilder.buildWithFields(iface, mParser.getVarDatabase())
            : mBuilder.buildWithOverrides(iface, mParser.getVarDatabase(),
                                          overrides);

        overrides.clear();
        for (const auto& f : pkt.fields) {
            overrides[f.first] = f.second;
        }

        round.emplace_back(iface.getName(), std::move(pkt));
    }

    return round;
}

Session::StackedPacket Session::generateStack(
    const std::vector<const ProtocolInterface*>& layers)
{
    StackedPacket result;
    result.layers.reserve(layers.size());

    for (const ProtocolInterface* iface : layers) {
        PacketBuilder::BuiltPacket pkt =
            mBuilder.buildWithFields(*iface, mParser.getVarDatabase());

        result.bytes.insert(result.bytes.end(),
                            pkt.bytes.begin(), pkt.bytes.end());
        result.layers.emplace_back(iface->getName(), std::move(pkt));
    }

    return result;
}

PacketBuilder::BuiltPacket Session::decode(
    const ProtocolInterface& iface, const std::vector<uint8_t>& bytes) const
{
    PacketDecoder decoder;
    return decoder.decode(iface, mParser.getVarDatabase(), bytes);
}

Session::StackedPacket Session::decodeStack(
    const std::vector<const ProtocolInterface*>& layers,
    const std::vector<uint8_t>& bytes) const
{
    StackedPacket result;
    result.bytes = bytes;
    result.layers.reserve(layers.size());

    PacketDecoder decoder;
    size_t offset = 0;

    for (const ProtocolInterface* iface : layers) {
        PacketBuilder::BuiltPacket layer =
            decoder.decode(*iface, mParser.getVarDatabase(), bytes, offset);
        offset += PacketDecoder::layerBytes(*iface, mParser.getVarDatabase());
        result.layers.emplace_back(iface->getName(), std::move(layer));
    }

    return result;
}

/* ---------------- logic-driven stack runtime ---------------- */

StackBuilderErr Session::loadStack(const std::string& stackYamlPath,
                                   std::unique_ptr<Stack>& outStack) const
{
    StackBuilder builder(mParser.getServiceDatabase());
    return builder.build(stackYamlPath, outStack);
}

const ProtocolInterface* Session::interfaceForLayer(
    const StackLayer& layer) const
{
    const Database<ProtocolInterface>& db = mParser.getInterfaceDatabase();

    /* Explicit interface wins. */
    if (!layer.interfaceName.empty()) {
        const std::string key = layer.serviceName + "::" +
                                layer.entityName + "::" + layer.interfaceName;
        return db.getElement(key);
    }

    /* Otherwise take the first interface under "service::entity::" (or
     * "service::" when the layer names no entity), in key order. */
    const std::string prefix = layer.entityName.empty()
        ? layer.serviceName + "::"
        : layer.serviceName + "::" + layer.entityName + "::";

    const ProtocolInterface* found = nullptr;
    db.forEach([&](const std::string& name, const ProtocolInterface& iface) {
        if (found == nullptr && name.rfind(prefix, 0) == 0) {
            found = &iface;
        }
    });
    return found;
}

void Session::populateContext(
    LayerContext& ctx, const ProtocolInterface& iface,
    const std::vector<std::pair<std::string, uint64_t>>& fieldValues) const
{
    const Database<Var>& varDb = mParser.getVarDatabase();

    /* Map each field short-name to its declared bit width. */
    ctx.clearFields();
    for (const auto& fv : fieldValues) {
        uint32_t bits = 0;
        for (const std::string& ref : iface.getVarRefs()) {
            const auto pos = ref.rfind("::");
            const std::string shortName =
                (pos != std::string::npos) ? ref.substr(pos + 2) : ref;
            if (shortName == fv.first) {
                const Var* var = varDb.getElement(ref);
                if (var) bits = var->getSize();
                break;
            }
        }
        if (bits != 0) {
            ctx.addField(fv.first, fv.second, bits);
        }
    }
}

Session::StackedPacket Session::generateStack(Stack& stack)
{
    StackedPacket result;

    const auto& layers = stack.layers();
    std::vector<const ProtocolInterface*> ifaces(layers.size(), nullptr);

    /* 1. Generate initial field values and load each layer's context. */
    for (size_t i = 0; i < layers.size(); ++i) {
        const ProtocolInterface* iface = interfaceForLayer(*layers[i]);
        ifaces[i] = iface;
        if (iface == nullptr) {
            log(LogLevel::warn) << "Session: no interface for stack layer '"
                                << layers[i]->serviceName << "'\n";
            continue;
        }
        PacketBuilder::BuiltPacket built =
            mBuilder.buildWithFields(*iface, mParser.getVarDatabase());
        populateContext(layers[i]->context, *iface, built.fields);
    }

    /* 2. Run logic: modules fill derived fields now that every layer's
     *    size and adjacency are known (bottom-up). */
    for (const auto& layer : layers) {
        layer->logic->onEncode(layer->context);
    }

    /* 3. Pack each layer from its (possibly mutated) context and concat. */
    for (const auto& layer : layers) {
        PacketBuilder::BuiltPacket lp;
        size_t bitOffset = 0;
        for (const LayerContext::Field& f : layer->context.fields()) {
            PacketBuilder::appendBits(lp.bytes, bitOffset, f.value, f.bits);
            lp.fields.emplace_back(f.name, f.value);
        }
        result.bytes.insert(result.bytes.end(),
                            lp.bytes.begin(), lp.bytes.end());
        result.layers.emplace_back(layer->serviceName, std::move(lp));
    }

    return result;
}

Session::StackedPacket Session::decodeStack(
    Stack& stack, const std::vector<uint8_t>& bytes)
{
    StackedPacket result;
    result.bytes = bytes;

    const auto& layers = stack.layers();
    PacketDecoder decoder;
    size_t offset = 0;

    for (size_t i = 0; i < layers.size(); ++i) {
        const ProtocolInterface* iface = interfaceForLayer(*layers[i]);
        if (iface == nullptr) {
            log(LogLevel::warn) << "Session: no interface for stack layer '"
                                << layers[i]->serviceName
                                << "' during decode\n";
            result.layers.emplace_back(layers[i]->serviceName,
                                       PacketBuilder::BuiltPacket{});
            continue;
        }

        PacketBuilder::BuiltPacket layer = decoder.decode(
            *iface, mParser.getVarDatabase(), bytes, offset);
        offset += PacketDecoder::layerBytes(*iface, mParser.getVarDatabase());

        populateContext(layers[i]->context, *iface, layer.fields);
        layers[i]->logic->onDecode(layers[i]->context);

        /* Demux check: the module's predicted upper service should match
         * the next layer actually present. Divergence is informational. */
        const std::string next = layers[i]->logic->nextLayer(
            layers[i]->context);
        if (!next.empty() && i + 1 < layers.size() &&
            next != layers[i + 1]->serviceName) {
            log(LogLevel::info) << "Session: layer '" << layers[i]->serviceName
                                << "' demux predicted '" << next
                                << "' but next layer is '"
                                << layers[i + 1]->serviceName << "'\n";
        }

        result.layers.emplace_back(layers[i]->serviceName, std::move(layer));
    }

    return result;
}

std::string Session::toHex(const std::vector<uint8_t>& bytes)
{
    static const char kHex[] = "0123456789abcdef";
    std::string s;
    s.reserve(bytes.size() * 2);
    for (uint8_t byte : bytes) {
        s += kHex[byte >> 4];
        s += kHex[byte & 0xF];
    }
    return s;
}

bool Session::fromHex(const std::string& hex, std::vector<uint8_t>& out)
{
    if (hex.size() % 2 != 0) {
        return false;
    }
    out.clear();
    out.reserve(hex.size() / 2);
    for (size_t i = 0; i < hex.size(); i += 2) {
        const std::string byte = hex.substr(i, 2);
        char* end = nullptr;
        const unsigned long v = std::strtoul(byte.c_str(), &end, 16);
        if (end != byte.c_str() + 2) {
            return false;
        }
        out.push_back(static_cast<uint8_t>(v));
    }
    return true;
}

std::string Session::toJson(const PacketBuilder::BuiltPacket& pkt)
{
    std::string s = "{\"fields\":{";
    for (size_t i = 0; i < pkt.fields.size(); ++i) {
        if (i) {
            s += ',';
        }
        s += '"';
        s += pkt.fields[i].first;
        s += "\":";
        s += std::to_string(pkt.fields[i].second);
    }
    s += "},\"hex\":\"";
    s += toHex(pkt.bytes);
    s += "\"}";
    return s;
}

std::string Session::toJson(const StackedPacket& pkt)
{
    std::string s = "{\"hex\":\"";
    s += toHex(pkt.bytes);
    s += "\",\"layers\":";
    s += toJson(pkt.layers);
    s += "}";
    return s;
}

std::string Session::toJson(const std::vector<Layer>& chain)
{
    std::string s = "[";
    for (size_t i = 0; i < chain.size(); ++i) {
        if (i) {
            s += ',';
        }
        s += "{\"iface\":\"";
        s += chain[i].first;
        s += "\",";
        s += toJson(chain[i].second).substr(1); /* strip leading '{' */
    }
    s += ']';
    return s;
}

} /* namespace tsn */
