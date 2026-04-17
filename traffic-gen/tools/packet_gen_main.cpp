/*
 * SPDX-FileCopyrightText: 2025 Kebag-Logic (https://kebag-logic.com)
 * SPDX-FileCopyrightText: 2025 Alexandre Malki <alexandre.malki@kebag-logic.com>
 * SPDX-License-Identifier: MIT
 *
 * packet_gen — generate AECP / ADP packets from YAML protocol definitions.
 *
 * Usage:
 *   packet_gen --yaml-dir <dir> --interface <qualified_name> [options]
 *
 * Options:
 *   --yaml-dir  <path>   Directory containing protocol YAML files (required)
 *   --interface <name>   Fully qualified interface name (required)
 *                        e.g. "atdecc_aecp_acquire_entity::AECP_ACQUIRE_ENTITY::AECP_ACQUIRE_ENTITY_IF"
 *   --count     <n>      Number of packets to generate (default: 1)
 *   --seed      <n>      PRNG seed (default: random)
 *   --output    hex|json Output format written to stdout (default: json)
 *                        json: one JSON object per line with "hex" and "fields" keys
 *                        hex:  one hex string per line
 *   --transport <spec>   Where to also deliver packets (default: none)
 *                        none
 *                        raw:<ifname>       Linux AF_PACKET raw socket
 *                        pcap:<filepath>    Write libpcap-format capture file
 *                        verilator:<socket> AXI-Stream over UNIX socket
 *   --help               Print this help
 *
 * Exit codes: 0 = success, 1 = argument error, 2 = parse/send error.
 */

#include <packet_builder.h>
#include <pcap_sender.h>
#include <protocol_parser.h>
#include <raw_socket_sender.h>
#include <verilator_sender.h>

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <memory>
#include <random>
#include <string>
#include <vector>

/* ------------------------------------------------------------------ */
/*  Minimal argument parser                                            */
/* ------------------------------------------------------------------ */

struct Args {
    std::string yamlDir;
    std::string iface;
    size_t      count     = 1;
    uint64_t    seed      = 0;
    bool        seedSet   = false;
    std::string output    = "json";   /* hex | json */
    std::string transport = "none";
};

static void printUsage(const char* prog)
{
    std::cerr
        << "Usage: " << prog << " --yaml-dir <dir> --interface <name> [options]\n"
        << "  --count     <n>      packets to generate (default 1)\n"
        << "  --seed      <n>      PRNG seed (default random)\n"
        << "  --output    hex|json stdout format (default json)\n"
        << "  --transport none|raw:<if>|pcap:<file>|verilator:<sock>  (default none)\n"
        << "  --help\n";
}

static bool parseArgs(int argc, char* argv[], Args& out)
{
    for (int i = 1; i < argc; ++i) {
        const std::string a = argv[i];
        auto nextVal = [&]() -> std::string {
            if (i + 1 >= argc) {
                std::cerr << "Missing value for " << a << "\n";
                return {};
            }
            return argv[++i];
        };

        if (a == "--yaml-dir")   { out.yamlDir   = nextVal(); }
        else if (a == "--interface") { out.iface  = nextVal(); }
        else if (a == "--count")   {
            const std::string v = nextVal();
            if (!v.empty()) out.count = static_cast<size_t>(std::stoull(v));
        }
        else if (a == "--seed") {
            const std::string v = nextVal();
            if (!v.empty()) { out.seed = std::stoull(v); out.seedSet = true; }
        }
        else if (a == "--output")    { out.output    = nextVal(); }
        else if (a == "--transport") { out.transport = nextVal(); }
        else if (a == "--help") { printUsage(argv[0]); return false; }
        else {
            std::cerr << "Unknown option: " << a << "\n";
            return false;
        }
    }

    if (out.yamlDir.empty() || out.iface.empty()) {
        std::cerr << "--yaml-dir and --interface are required\n";
        return false;
    }
    if (out.output != "hex" && out.output != "json") {
        std::cerr << "--output must be 'hex' or 'json'\n";
        return false;
    }
    return true;
}

/* ------------------------------------------------------------------ */
/*  Transport factory                                                  */
/* ------------------------------------------------------------------ */

static std::unique_ptr<ISender> makeSender(const std::string& spec)
{
    if (spec == "none" || spec.empty()) return nullptr;

    const auto colon = spec.find(':');
    if (colon == std::string::npos) return nullptr;

    const std::string kind  = spec.substr(0, colon);
    const std::string param = spec.substr(colon + 1);

    if (kind == "raw")       return std::make_unique<RawSocketSender>(param);
    if (kind == "pcap")      return std::make_unique<PcapSender>(param);
    if (kind == "verilator") return std::make_unique<VerilatorSender>(param);

    std::cerr << "Unknown transport kind: " << kind << "\n";
    return nullptr;
}

/* ------------------------------------------------------------------ */
/*  Hex encoding                                                       */
/* ------------------------------------------------------------------ */

static std::string toHex(const std::vector<uint8_t>& b)
{
    static const char kHex[] = "0123456789abcdef";
    std::string s;
    s.reserve(b.size() * 2);
    for (uint8_t byte : b) {
        s += kHex[byte >> 4];
        s += kHex[byte & 0xF];
    }
    return s;
}

/* ------------------------------------------------------------------ */
/*  JSON serialisation (no external library)                          */
/* ------------------------------------------------------------------ */

static std::string toJson(const PacketBuilder::BuiltPacket& pkt)
{
    std::string s = "{\"fields\":{";
    for (size_t i = 0; i < pkt.fields.size(); ++i) {
        if (i) s += ',';
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

/* ------------------------------------------------------------------ */
/*  main                                                               */
/* ------------------------------------------------------------------ */

int main(int argc, char* argv[])
{
    Args args;
    if (!parseArgs(argc, argv, args)) {
        printUsage(argv[0]);
        return 1;
    }

    /* Parse YAML directory. */
    ProtocolParser parser(args.yamlDir);
    const auto parseErr = parser.parse();
    if (parseErr.getErrorCode() != ProtocolParserErr::PROTOPARSER_SUCCESS) {
        std::cerr << "Parser error: " << parseErr.getErrorMsg() << "\n";
        return 2;
    }

    /* Resolve interface. */
    const ProtocolInterface* iface =
        parser.getInterfaceDatabase().getElement(args.iface);
    if (!iface) {
        std::cerr << "Interface not found: " << args.iface << "\n";
        return 2;
    }

    /* Set up sender (optional). */
    std::unique_ptr<ISender> sender = makeSender(args.transport);
    if (sender) {
        const auto err = sender->open();
        if (err.getErrorCode() != SenderErr::SENDER_SUCCESS) {
            std::cerr << "Failed to open transport: " << args.transport << "\n";
            return 2;
        }
    }

    /* Set up builder. */
    PacketBuilder builder;
    if (args.seedSet) {
        builder.seed(args.seed);
    } else {
        std::random_device rd;
        builder.seed(rd());
    }

    /* Generate packets. */
    for (size_t i = 0; i < args.count; ++i) {
        auto pkt = builder.buildWithFields(*iface, parser.getVarDatabase());

        if (args.output == "json") {
            std::cout << toJson(pkt) << '\n';
        } else {
            std::cout << toHex(pkt.bytes) << '\n';
        }

        if (sender) {
            const auto err = sender->send(pkt.bytes);
            if (err.getErrorCode() != SenderErr::SENDER_SUCCESS) {
                std::cerr << "Send failed on packet " << i << "\n";
                sender->close();
                return 2;
            }
        }
    }

    if (sender) sender->close();
    return 0;
}
