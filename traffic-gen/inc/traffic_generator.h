/*
 * SPDX-FileCopyrightText: 2025 Kebag-Logic (https://kebag-logic.com)
 * SPDX-FileCopyrightText: 2025 Alexandre Malki <alexandre.malki@kebag-logic.com>
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <memory>
#include <string>
#include <vector>

#include <error.h>
#include <packet_builder.h>
#include <protocol_interface.h>
#include <protocol_parser.h>
#include <sender.h>

class TrafficGeneratorErr : public Error {
public:
    enum {
        TGEN_SUCCESS,
        TGEN_ERR_UNKNOWN,
        TGEN_ERR_NOT_OPEN,
        TGEN_ERR_SEND_FAILED,
        TGEN_ERR_IFACE_NOT_FOUND,
        TGEN_ERR_EMPTY_PACKET,

        TGEN_ERR_ENUM_SENTINEL
    };

    TrafficGeneratorErr() = delete;
    explicit TrafficGeneratorErr(int code) : Error(code, mErrVector) {}

    TrafficGeneratorErr& operator=(const TrafficGeneratorErr other)
    {
        mErrorCode = other.mErrorCode;
        return *this;
    }

private:
    const std::vector<std::string> mErrVector = {
        "TGEN: SUCCESS",
        "TGEN: ERR: UNKNOWN",
        "TGEN: ERR: NOT OPEN",
        "TGEN: ERR: SEND FAILED",
        "TGEN: ERR: INTERFACE NOT FOUND",
        "TGEN: ERR: EMPTY PACKET",
    };
};

/**
 * @brief Drives packet generation from parsed protocol definitions.
 *
 * A TrafficGenerator ties together:
 *  - A ProtocolParser (source of Var and ProtocolInterface databases)
 *  - A PacketBuilder  (builds byte buffers from var constraints)
 *  - An ISender       (delivers the bytes — raw socket or Verilator)
 *
 * Typical flow:
 * @code
 *   ProtocolParser parser("/path/to/protocols");
 *   parser.parse();
 *
 *   auto sender = std::make_unique<RawSocketSender>("eth0");
 *   // — or —
 *   auto sender = std::make_unique<VerilatorSender>("/tmp/vrl.sock");
 *
 *   TrafficGenerator gen(parser, std::move(sender));
 *   gen.open();
 *   gen.sendFiltered(ProtocolInterface::IN);   // feed all IN interfaces
 *   gen.close();
 * @endcode
 */
class TrafficGenerator {
public:
    /**
     * @param parser  Fully initialised parser (parse() already called).
     * @param sender  Heap-allocated sender; ownership is transferred.
     */
    TrafficGenerator(const ProtocolParser& parser,
                     std::unique_ptr<ISender> sender);

    /** Open the underlying sender. */
    const TrafficGeneratorErr open();

    /** Close the underlying sender. */
    void close() noexcept;

    /**
     * @brief Send one packet for every interface whose direction matches
     *        @p dir.  Interfaces with no resolvable vars produce an empty
     *        packet and are skipped (TGEN_ERR_EMPTY_PACKET is not fatal here).
     *
     * @param dir  ProtocolInterface::IN or ProtocolInterface::OUT
     */
    const TrafficGeneratorErr sendFiltered(ProtocolInterface::IfDirections dir);

    /**
     * @brief Send one packet for each interface regardless of direction.
     */
    const TrafficGeneratorErr sendAll();

    /**
     * @brief Send one packet for the named interface.
     *
     * @param qualifiedName  Fully qualified interface name, e.g.
     *                       "simple_service::SIMPLE_SERVICE_ENTITY::SIMPLE_INTERFACE"
     */
    const TrafficGeneratorErr send(const std::string& qualifiedName);

    /**
     * @brief Repeatedly send packets for @p qualifiedName.
     *
     * @param qualifiedName  Fully qualified interface name.
     * @param count          Number of packets to send (0 = unlimited).
     */
    const TrafficGeneratorErr sendLoop(const std::string& qualifiedName,
                                       size_t count);

    /** Seed the packet builder's PRNG (useful for reproducible tests). */
    void seedRng(uint64_t s);

private:
    const TrafficGeneratorErr sendOne(const ProtocolInterface& iface);

    const ProtocolParser& mParser;
    std::unique_ptr<ISender> mSender;
    PacketBuilder mBuilder;
    bool mOpen;
};
