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
#include <unordered_map>
#include <vector>

/* ------------------------------------------------------------------ */
/*  Minimal argument parser                                            */
/* ------------------------------------------------------------------ */

struct Args {
    std::string yamlDir;
    std::string iface;
    size_t      count          = 1;
    uint64_t    seed           = 0;
    bool        seedSet        = false;
    std::string output         = "json";   /* hex | json */
    std::string transport      = "none";
    bool        listInterfaces = false;
    /* Ordered chain of interface names supplied via --connect A:B:C */
    std::vector<std::string> chain;
};

static void printUsage(const char* prog)
{
    std::cerr
        << "\n"
        << "packet_gen — IEEE 1722.1 / ADP packet generator driven by YAML protocol definitions\n"
        << "\n"
        << "USAGE\n"
        << "  " << prog << " --yaml-dir <dir> --interface <name> [options]\n"
        << "\n"
        << "REQUIRED\n"
        << "  --yaml-dir <path>\n"
        << "      Directory (searched recursively) containing protocol YAML files.\n"
        << "      Every .yaml file must declare a 'service:' key at the top level.\n"
        << "      Example: protocols/application/1722_1/aecp\n"
        << "\n"
        << "  --interface <qualified-name>\n"
        << "      Fully qualified interface name in the form:\n"
        << "        <service>::<ENTITY>::<INTERFACE>\n"
        << "      Example: atdecc_aecp_acquire_entity::AECP_ACQUIRE_ENTITY::AECP_ACQUIRE_ENTITY_IF\n"
        << "      Tip: run with --list-interfaces to discover available names.\n"
        << "\n"
        << "OPTIONS\n"
        << "  --count <n>          Number of packets to generate.  Default: 1.\n"
        << "                       Use 0 for an unlimited stream (Ctrl-C to stop).\n"
        << "\n"
        << "  --seed <n>           Seed the internal Mersenne-Twister PRNG with the\n"
        << "                       given 64-bit integer.  Identical seeds produce\n"
        << "                       identical packet sequences — useful for reproducible\n"
        << "                       fuzzing and regression tests.  Default: random.\n"
        << "\n"
        << "  --output hex|json    Format for packets written to stdout.  Default: json.\n"
        << "    hex                One lower-case hex string per line, no delimiters.\n"
        << "                       Example: 1402422e2c43f8a1...\n"
        << "    json               One JSON object per line (newline-delimited JSON):\n"
        << "                         {\"fields\":{\"message_type\":0,\"status\":0,...},\n"
        << "                          \"hex\":\"1402422e...\"}\n"
        << "                       Fields appear in YAML declaration order.\n"
        << "                       Integer field values are decimal.\n"
        << "\n"
        << "  --transport <spec>   Deliver each generated packet via the named transport\n"
        << "                       in addition to writing to stdout.  Default: none.\n"
        << "    none               Stdout only — no hardware or file I/O.\n"
        << "    raw:<ifname>       Inject directly onto a network interface using a\n"
        << "                       Linux AF_PACKET/SOCK_RAW socket.  Requires\n"
        << "                       CAP_NET_RAW (run as root or grant the capability).\n"
        << "                       Example: raw:eth0\n"
        << "    pcap:<filepath>    Write a libpcap-format capture file (LINKTYPE_ETHERNET,\n"
        << "                       no libpcap dependency).  Compatible with Wireshark and\n"
        << "                       tcpdump.  The file is created or truncated on open.\n"
        << "                       Example: pcap:/tmp/aecp_capture.pcap\n"
        << "    verilator:<socket> Send AXI-Stream beats over a UNIX domain socket to a\n"
        << "                       Verilator DUT.  The socket path must already exist\n"
        << "                       (started by the simulation).\n"
        << "                       Example: verilator:/tmp/verilator_axis.sock\n"
        << "\n"
        << "  --list-interfaces    Parse --yaml-dir and print all available interface\n"
        << "                       names, then exit.  Useful for discovering what\n"
        << "                       --interface values are valid in a given directory.\n"
        << "\n"
        << "  --connect <A:B[:C…]> Chain two or more interfaces into a single round.\n"
        << "                       Each round generates one packet per interface in\n"
        << "                       the declared order.  Fields shared by name between\n"
        << "                       consecutive packets are propagated forward:\n"
        << "                         • B receives A's field values for any field name\n"
        << "                           that B does not fix via 'expected.value/values'.\n"
        << "                         • C receives B's field values the same way.\n"
        << "                       Typical use: simulate a command/response exchange\n"
        << "                       where the responder echoes sequence_id,\n"
        << "                       target_entity_id, controller_entity_id, etc.\n"
        << "                       When --connect is given, --interface is optional\n"
        << "                       (the chain defines all interfaces to use).\n"
        << "                       Multiple --connect flags append to the same chain.\n"
        << "                       Example:\n"
        << "                         --connect CMD_IF:RSP_IF\n"
        << "                         --connect A:B:C  (three-step chain)\n"
        << "\n"
        << "  --help               Print this message and exit.\n"
        << "\n"
        << "EXIT CODES\n"
        << "  0  Success.\n"
        << "  1  Argument error (bad option, missing required argument).\n"
        << "  2  Runtime error (parse failure, interface not found, send error).\n"
        << "\n"
        << "EXAMPLES\n"
        << "  # Generate 1 JSON packet from the ACQUIRE_ENTITY YAML:\n"
        << "  " << prog << " \\\n"
        << "    --yaml-dir protocols/application/1722_1/aecp \\\n"
        << "    --interface atdecc_aecp_acquire_entity::AECP_ACQUIRE_ENTITY::AECP_ACQUIRE_ENTITY_IF\n"
        << "\n"
        << "  # Reproducible fuzz run — 1000 hex packets, seed 42:\n"
        << "  " << prog << " \\\n"
        << "    --yaml-dir protocols/application/1722_1/aecp \\\n"
        << "    --interface atdecc_aecp_acquire_entity::AECP_ACQUIRE_ENTITY::AECP_ACQUIRE_ENTITY_IF \\\n"
        << "    --count 1000 --seed 42 --output hex\n"
        << "\n"
        << "  # Capture 50 VENDOR_UNIQUE PDUs into a pcap file:\n"
        << "  " << prog << " \\\n"
        << "    --yaml-dir protocols/application/1722_1/aecp \\\n"
        << "    --interface atdecc_aecp_vendor_unique::AECP_VENDOR_UNIQUE::AECP_VENDOR_UNIQUE_IF \\\n"
        << "    --count 50 --transport pcap:/tmp/vendor_unique.pcap\n"
        << "\n"
        << "  # Inject 10 ADP PDUs onto eth0 (requires root):\n"
        << "  " << prog << " \\\n"
        << "    --yaml-dir parser/tests/test_resources/0020_adp_protocol_test \\\n"
        << "    --interface atdecc_adp_service::ADP_INTERFACE::ADP_IF_DATA \\\n"
        << "    --count 10 --transport raw:eth0\n"
        << "\n"
        << "  # List all interfaces defined in the AECP YAML directory:\n"
        << "  " << prog << " \\\n"
        << "    --yaml-dir protocols/application/1722_1/aecp --list-interfaces\n"
        << "\n"
        << "  # Simulate an ACQUIRE_ENTITY command/response exchange (5 rounds):\n"
        << "  " << prog << " \\\n"
        << "    --yaml-dir protocols/application/1722_1/aecp \\\n"
        << "    --connect atdecc_aecp_acquire_entity::AECP_ACQUIRE_ENTITY::AECP_ACQUIRE_ENTITY_IF\\\n"
        << "             :atdecc_aecp_acquire_entity::AECP_ACQUIRE_ENTITY::AECP_ACQUIRE_ENTITY_IF \\\n"
        << "    --count 5\n"
        << "\n";
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
        else if (a == "--output")          { out.output         = nextVal(); }
        else if (a == "--transport")       { out.transport      = nextVal(); }
        else if (a == "--list-interfaces") { out.listInterfaces = true; }
        else if (a == "--connect") {
            /* Split colon-separated chain and append to out.chain.
               Guard against '::' inside qualified names by splitting only on
               single colons that are NOT part of '::'. */
            std::string spec = nextVal();
            /* Replace '::' with a placeholder, split on ':', restore. */
            static const std::string kSep = "\x01";
            std::string s = spec;
            for (size_t p = 0; (p = s.find("::", p)) != std::string::npos; )
                { s.replace(p, 2, kSep); p += kSep.size(); }
            std::string seg;
            for (char c : s + ":") {
                if (c == ':') {
                    /* Restore '::' and push segment. */
                    for (size_t p = 0;
                         (p = seg.find(kSep, p)) != std::string::npos; )
                        { seg.replace(p, kSep.size(), "::"); p += 2; }
                    if (!seg.empty()) out.chain.push_back(seg);
                    seg.clear();
                } else {
                    seg += c;
                }
            }
        }
        else if (a == "--help") { printUsage(argv[0]); std::exit(0); }
        else {
            std::cerr << "Unknown option: " << a << "\n";
            return false;
        }
    }

    if (out.yamlDir.empty()) {
        std::cerr << "--yaml-dir is required\n";
        return false;
    }
    /* --interface is optional when --connect supplies the chain. */
    if (!out.listInterfaces && out.iface.empty() && out.chain.empty()) {
        std::cerr << "--interface (or --connect) is required\n";
        return false;
    }
    /* If --interface is also given alongside --connect, prepend it. */
    if (!out.iface.empty() && !out.chain.empty()) {
        out.chain.insert(out.chain.begin(), out.iface);
        out.iface.clear();
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

    /* --list-interfaces: dump all known interface names and exit. */
    if (args.listInterfaces) {
        std::cout << "Available interfaces in '" << args.yamlDir << "':\n";
        parser.getInterfaceDatabase().forEach(
            [](const std::string& name, const ProtocolInterface& ifc) {
                const char* dir =
                    (ifc.getDirection() == ProtocolInterface::IN) ? "in" : "out";
                std::cout << "  " << name << "  [" << dir << "]\n";
            });
        return 0;
    }

    /* Build the ordered list of interfaces to use each round. */
    std::vector<const ProtocolInterface*> pipeline;
    const auto resolveIface = [&](const std::string& name) -> bool {
        const ProtocolInterface* p =
            parser.getInterfaceDatabase().getElement(name);
        if (!p) {
            std::cerr << "Interface not found: " << name << "\n";
            std::cerr << "Run with --list-interfaces to see available names.\n";
            return false;
        }
        pipeline.push_back(p);
        return true;
    };

    if (!args.chain.empty()) {
        for (const auto& name : args.chain)
            if (!resolveIface(name)) return 2;
    } else {
        if (!resolveIface(args.iface)) return 2;
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

    const bool isChain     = pipeline.size() > 1;
    const bool jsonOut     = (args.output == "json");

    /* Generate packets — one round = one packet per pipeline stage. */
    for (size_t round = 0; round < args.count; ++round) {
        std::unordered_map<std::string, uint64_t> overrides;

        if (isChain && jsonOut) std::cout << "[";

        for (size_t stage = 0; stage < pipeline.size(); ++stage) {
            const ProtocolInterface* iface = pipeline[stage];

            PacketBuilder::BuiltPacket pkt =
                (stage == 0)
                ? builder.buildWithFields(*iface, parser.getVarDatabase())
                : builder.buildWithOverrides(*iface, parser.getVarDatabase(),
                                             overrides);

            /* Build overrides for the next stage from this packet's fields. */
            overrides.clear();
            for (const auto& f : pkt.fields)
                overrides[f.first] = f.second;

            /* Output. */
            if (jsonOut) {
                if (isChain) {
                    /* Embed iface name so caller can tell stages apart. */
                    std::string s = "{\"iface\":\"";
                    s += iface->getName();
                    s += "\",";
                    s += toJson(pkt).substr(1); /* strip leading '{' */
                    std::cout << (stage ? "," : "") << s;
                } else {
                    std::cout << toJson(pkt) << '\n';
                }
            } else {
                std::cout << toHex(pkt.bytes) << '\n';
            }

            if (sender) {
                const auto err = sender->send(pkt.bytes);
                if (err.getErrorCode() != SenderErr::SENDER_SUCCESS) {
                    std::cerr << "Send failed on round " << round
                              << " stage " << stage << "\n";
                    sender->close();
                    return 2;
                }
            }
        }

        if (isChain && jsonOut) std::cout << "]\n";
    }

    if (sender) sender->close();
    return 0;
}
