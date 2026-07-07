/*
 * SPDX-FileCopyrightText: 2026 Kebag-Logic (https://kebag-logic.com)
 * SPDX-FileCopyrightText: 2026 Alexandre Malki <alexandre.malki@kebag-logic.com>
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include <memory>

#include <tsn/packet_builder.h>
#include <tsn/packet_decoder.h>
#include <tsn/protocol_parser.h>
#include <tsn/stack.h>
#include <tsn/stack_builder.h>

namespace tsn {

/**
 * @brief High-level facade over the parser + builder/decoder pipeline.
 *
 *  One Session owns a parsed protocol directory, a seeded PRNG and the
 *  encode/decode entry points that used to live only in the packet_gen
 *  CLI: single-interface generation, --connect style field-propagation
 *  chains, --stack style layer concatenation, and the reverse decode
 *  paths. Embed this class to drive tsn-gen from another application;
 *  packet_gen itself is a thin client of it.
 */
class Session {
public:
    /** One named layer of a stacked frame. */
    using Layer = std::pair<std::string, PacketBuilder::BuiltPacket>;

    /** A multi-layer frame: concatenated bytes plus the per-layer slices. */
    struct StackedPacket {
        std::vector<uint8_t> bytes;
        std::vector<Layer> layers;
    };

    explicit Session(std::string protocolDir);

    /**
     * @brief Parse every protocol YAML under the directory given at
     *        construction. Diagnostics go to tsn::log().
     * @return true when at least one valid protocol was loaded.
     */
    bool parse();

    const ProtocolParser& parser() const { return mParser; }

    /** Seed the packet generator PRNG for reproducible sequences. */
    void seed(uint64_t s) { mBuilder.seed(s); }

    /** @return the interface for a fully qualified name, or nullptr. */
    const ProtocolInterface* findInterface(const std::string& name) const;

    /* ---------------- generation ---------------- */

    /** Generate one packet for @p iface (random within YAML constraints). */
    PacketBuilder::BuiltPacket generate(const ProtocolInterface& iface);

    /**
     * @brief Command/response chain: one packet per stage, in order.
     *        Fields of stage N propagate into any same-named field of
     *        stage N+1 that the YAML does not fix via expected values.
     */
    std::vector<Layer> generateChain(
        const std::vector<const ProtocolInterface*>& stages);

    /**
     * @brief Layer stack: each interface generated independently and the
     *        byte buffers concatenated bottom-up into one on-wire frame.
     *        No logic modules run — fields keep their generated values.
     */
    StackedPacket generateStack(
        const std::vector<const ProtocolInterface*>& layers);

    /* ---------------- logic-driven stack runtime ---------------- */

    /**
     * @brief Build a bound Stack from a stack YAML file, resolving each
     *        layer's service + logic module against this session's parse.
     * @param [out] outStack holds the built stack on success.
     */
    StackBuilderErr loadStack(const std::string& stackYamlPath,
                              std::unique_ptr<Stack>& outStack) const;

    /**
     * @brief Generate one frame through the stack's logic modules:
     *        populate each layer's fields, run onEncode (bottom-up) so
     *        modules fill derived fields (length, demux selectors, …),
     *        then pack. Produces semantically valid frames where a raw
     *        --stack concatenation would not.
     */
    StackedPacket generateStack(Stack& stack);

    /**
     * @brief Decode one frame through the stack: slice each layer, run
     *        onDecode, and check each module's nextLayer() demux against
     *        the actual upper layer (mismatches are logged, not fatal).
     */
    StackedPacket decodeStack(Stack& stack,
                              const std::vector<uint8_t>& bytes);

    /* ---------------- decode ---------------- */

    /** Decode @p bytes against a single interface layout. */
    PacketBuilder::BuiltPacket decode(const ProtocolInterface& iface,
                                      const std::vector<uint8_t>& bytes) const;

    /**
     * @brief Slice @p bytes at each layer's byte offset and decode the
     *        layers in order. StackedPacket::bytes keeps the full input.
     */
    StackedPacket decodeStack(
        const std::vector<const ProtocolInterface*>& layers,
        const std::vector<uint8_t>& bytes) const;

    /* ---------------- serialisation ---------------- */

    static std::string toHex(const std::vector<uint8_t>& bytes);

    /** @return false when @p hex is not an even-length hex string. */
    static bool fromHex(const std::string& hex, std::vector<uint8_t>& out);

    /** {"fields":{...},"hex":"..."} — field order = YAML order. */
    static std::string toJson(const PacketBuilder::BuiltPacket& pkt);

    /** {"hex":"...","layers":[{"iface":"...","fields":{...},"hex":"..."}]} */
    static std::string toJson(const StackedPacket& pkt);

    /** [{"iface":"...","fields":{...},"hex":"..."}, ...] — one chain round. */
    static std::string toJson(const std::vector<Layer>& chain);

private:
    /**
     * @brief Resolve the interface a stack layer generates/decodes with:
     *        the layer's explicit `interface:` when set, otherwise the
     *        first interface of "service::entity" in key order.
     * @return nullptr when no matching interface exists.
     */
    const ProtocolInterface* interfaceForLayer(const StackLayer& layer) const;

    /** Load a context's fields (name, value, bit width) from a built or
     *  decoded packet's field values, taking bit widths from the varDb. */
    void populateContext(LayerContext& ctx, const ProtocolInterface& iface,
                         const std::vector<std::pair<std::string, uint64_t>>&
                             fieldValues) const;

    std::string mProtocolDir;
    ProtocolParser mParser;
    PacketBuilder mBuilder;
};

} /* namespace tsn */
