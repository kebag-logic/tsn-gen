/*
 * SPDX-FileCopyrightText: 2025 Kebag-Logic (https://kebag-logic.com)
 * SPDX-FileCopyrightText: 2025 Alexandre Malki <alexandre.malki@kebag-logic.com>
 * SPDX-License-Identifier: MIT
 *
 * packet_gen — generate and decode packets from YAML protocol definitions.
 *
 * Thin CLI over tsn::Session (generation/decode/serialisation) and the
 * ISender / IReceiver transports. All diagnostics go to stderr (see
 * --log-level); stdout carries only NDJSON or hex packet lines.
 *
 * Exit codes: 0 = success, 1 = argument error, 2 = parse/send error.
 */

#include <tsn/log.h>
#include <tsn/pcap_receiver.h>
#include <tsn/pcap_sender.h>
#include <tsn/raw_socket_receiver.h>
#include <tsn/raw_socket_sender.h>
#include <tsn/session.h>
#include <tsn/validator.h>
#include <tsn/verilator_receiver.h>
#include <tsn/verilator_sender.h>
#include <tsn/version.h>

using namespace tsn;

#include <cstdint>
#include <cstdlib>
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
    size_t      count          = 1;
    uint64_t    seed           = 0;
    bool        seedSet        = false;
    std::string output         = "json";   /* hex | json */
    std::string transport      = "none";
    std::string logLevel       = "warn";
    bool        listInterfaces = false;
    bool        validate       = false;    /* --validate: schema-check YAML */
    bool        decode         = false;    /* --decode: receive + decode mode */
    std::string hexInput;                  /* --hex <hexstring>: direct decode */
    /* Ordered chain of interface names supplied via --connect A:B:C */
    std::vector<std::string> chain;
    /* Ordered layer interfaces supplied via --stack A:B:C (byte concatenation) */
    std::vector<std::string> stack;
    /* Path to a stack YAML supplied via --stack-file (logic-driven codec) */
    std::string stackFile;
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
        << "  --validate           Schema-check every YAML under --yaml-dir (and the\n"
        << "                       --stack-file if given) against the protocol/stack\n"
        << "                       schema, report file:line:col diagnostics on stderr,\n"
        << "                       then exit.  Exit code 0 = clean, 2 = errors found.\n"
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
        << "  --stack <A:B[:C…]>   Concatenate bytes from multiple protocol layers into\n"
        << "                       a single on-wire frame.  Each interface is generated\n"
        << "                       independently (no field propagation) and its byte\n"
        << "                       buffer is appended in declaration order.\n"
        << "                       Typical use: assemble a complete Ethernet frame:\n"
        << "                         Ethernet MAC header (14 B)\n"
        << "                         + AVTP control header (2 B)\n"
        << "                         + AECP PDU (39 B)\n"
        << "                       Use --yaml-dir pointing at the protocols root so all\n"
        << "                       layer YAMLs (data_link, application, …) are loaded.\n"
        << "                       JSON output per round:\n"
        << "                         {\"hex\":\"<all layers concatenated>\",\n"
        << "                          \"layers\":[{\"iface\":\"...\",\"hex\":\"...\",\n"
        << "                                     \"fields\":{…}}, …]}\n"
        << "                       --stack and --connect are mutually exclusive.\n"
        << "                       Example:\n"
        << "                         --stack ethernet_mac_frame::ETHERNET_FRAME::ETHERNET_FRAME_IF\\\n"
        << "                                :avtp_control_header::AVTP_CONTROL::AVTP_CONTROL_IF\\\n"
        << "                                :atdecc_aecp_acquire_entity::AECP_ACQUIRE_ENTITY::AECP_ACQUIRE_ENTITY_IF\n"
        << "\n"
        << "  --stack-file <path>  Assemble (or decode) a frame through a stack YAML and\n"
        << "                       its per-layer logic modules. Unlike --stack, each\n"
        << "                       layer's logic runs: derived fields (lengths, demux\n"
        << "                       selectors) are filled on encode and validated on\n"
        << "                       decode, producing semantically valid frames.\n"
        << "                       The stack YAML lists services bottom-up; see\n"
        << "                       stacks/ for examples.  Mutually exclusive with\n"
        << "                       --stack / --connect.\n"
        << "\n"
        << "  --decode             Receive packets (see --transport) or decode --hex, and\n"
        << "                       print the fields recovered against the declared\n"
        << "                       interface or stack layout.\n"
        << "\n"
        << "  --hex <hexstring>    Decode a single hex string and exit (implies --decode).\n"
        << "\n"
        << "  --log-level <lvl>    Diagnostic verbosity on stderr: quiet, error, warn,\n"
        << "                       info, debug.  Default: warn.  stdout stays reserved\n"
        << "                       for packet output in every mode.\n"
        << "\n"
        << "  --version            Print the tool version and exit.\n"
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
        << "\n"
        << "  # Assemble a complete on-wire IEEE 1722.1 AECP frame (Eth+AVTP+AECP = 55 bytes):\n"
        << "  " << prog << " \\\n"
        << "    --yaml-dir protocols \\\n"
        << "    --stack ethernet_mac_frame::ETHERNET_FRAME::ETHERNET_FRAME_IF\\\n"
        << "           :avtp_control_header::AVTP_CONTROL::AVTP_CONTROL_IF\\\n"
        << "           :atdecc_aecp_acquire_entity::AECP_ACQUIRE_ENTITY::AECP_ACQUIRE_ENTITY_IF \\\n"
        << "    --count 3 --seed 42\n"
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
        else if (a == "--stack-file")      { out.stackFile      = nextVal(); }
        else if (a == "--log-level")       { out.logLevel       = nextVal(); }
        else if (a == "--list-interfaces") { out.listInterfaces = true; }
        else if (a == "--validate")        { out.validate       = true; }
        else if (a == "--decode")          { out.decode        = true; }
        else if (a == "--hex")             { out.hexInput      = nextVal(); }
        else if (a == "--connect" || a == "--stack") {
            /* Split colon-separated list and append to out.chain / out.stack.
               Guard against '::' inside qualified names by splitting only on
               single colons that are NOT part of '::'. */
            std::string spec = nextVal();
            /* Replace '::' with a placeholder, split on ':', restore. */
            static const std::string kSep = "\x01";
            std::string s = spec;
            for (size_t p = 0; (p = s.find("::", p)) != std::string::npos; )
                { s.replace(p, 2, kSep); p += kSep.size(); }
            std::vector<std::string>& target = (a == "--connect") ? out.chain : out.stack;
            std::string seg;
            for (char c : s + ":") {
                if (c == ':') {
                    for (size_t p = 0;
                         (p = seg.find(kSep, p)) != std::string::npos; )
                        { seg.replace(p, kSep.size(), "::"); p += 2; }
                    if (!seg.empty()) target.push_back(seg);
                    seg.clear();
                } else {
                    seg += c;
                }
            }
        }
        else if (a == "--version") {
            std::cout << "packet_gen " << PROTOCOL_PARSER_VERSION << "\n";
            std::exit(0);
        }
        else if (a == "--help") { printUsage(argv[0]); std::exit(0); }
        else {
            std::cerr << "Unknown option: " << a << "\n";
            return false;
        }
    }

    if (!tsn::setLogLevel(out.logLevel)) {
        std::cerr << "--log-level must be quiet|error|warn|info|debug\n";
        return false;
    }
    if (out.yamlDir.empty()) {
        std::cerr << "--yaml-dir is required\n";
        return false;
    }
    /* --stack-file drives its own logic-based pipeline; it does not mix
       with the interface-list flags. */
    const int pipelineFlags = (!out.chain.empty() ? 1 : 0) +
                              (!out.stack.empty() ? 1 : 0) +
                              (!out.stackFile.empty() ? 1 : 0);
    if (pipelineFlags > 1) {
        std::cerr << "--connect, --stack and --stack-file are mutually "
                     "exclusive\n";
        return false;
    }
    /* --interface is optional when a pipeline flag supplies the layers,
       or in list/validate modes that consume the directory directly. */
    if (!out.listInterfaces && !out.validate && out.iface.empty() &&
        out.chain.empty() && out.stack.empty() && out.stackFile.empty()) {
        std::cerr << "--interface (or --connect / --stack / --stack-file) "
                     "is required\n";
        return false;
    }
    /* --hex implies --decode */
    if (!out.hexInput.empty()) out.decode = true;
    /* If --interface is also given alongside --connect, prepend it. */
    if (!out.iface.empty() && !out.chain.empty()) {
        out.chain.insert(out.chain.begin(), out.iface);
        out.iface.clear();
    }
    /* If --interface is also given alongside --stack, prepend it. */
    if (!out.iface.empty() && !out.stack.empty()) {
        out.stack.insert(out.stack.begin(), out.iface);
        out.iface.clear();
    }
    if (out.output != "hex" && out.output != "json") {
        std::cerr << "--output must be 'hex' or 'json'\n";
        return false;
    }
    return true;
}

/* ------------------------------------------------------------------ */
/*  Transport factories                                                */
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

static std::unique_ptr<IReceiver> makeReceiver(const std::string& spec)
{
    if (spec == "none" || spec.empty()) return nullptr;

    const auto colon = spec.find(':');
    if (colon == std::string::npos) return nullptr;

    const std::string kind  = spec.substr(0, colon);
    const std::string param = spec.substr(colon + 1);

    if (kind == "raw")       return std::make_unique<RawSocketReceiver>(param);
    if (kind == "pcap")      return std::make_unique<PcapReceiver>(param);
    if (kind == "verilator") return std::make_unique<VerilatorReceiver>(param);

    std::cerr << "Unknown receiver transport: " << kind << "\n";
    return nullptr;
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

    /* --validate: schema-check the corpus (and stack file) and exit. Runs
       before parsing so a non-conforming file is reported, not skipped. */
    if (args.validate) {
        ProtocolValidator validator;
        std::vector<ValidationFinding> findings;
        validator.validateDirectory(args.yamlDir, findings);
        if (!args.stackFile.empty()) {
            validator.validateStackFile(args.stackFile, findings);
        }

        size_t errors = 0;
        for (const ValidationFinding& f : findings) {
            const char* sev =
                (f.severity == ValidationFinding::ERROR) ? "error" : "warning";
            if (f.severity == ValidationFinding::ERROR) ++errors;
            std::cerr << f.file << ':' << f.line << ':' << f.col << ": "
                      << sev << ": " << f.message << '\n';
        }
        std::cerr << "validate: " << errors << " error(s) in '"
                  << args.yamlDir << "'"
                  << (args.stackFile.empty() ? "" : " + stack file") << '\n';
        return errors == 0 ? 0 : 2;
    }

    Session session(args.yamlDir);
    if (!session.parse()) {
        return 2;
    }

    /* --list-interfaces: dump all known interface names and exit. */
    if (args.listInterfaces) {
        std::cout << "Available interfaces in '" << args.yamlDir << "':\n";
        session.parser().getInterfaceDatabase().forEach(
            [](const std::string& name, const ProtocolInterface& ifc) {
                const char* dir =
                    (ifc.getDirection() == ProtocolInterface::IN) ? "in" : "out";
                std::cout << "  " << name << "  [" << dir << "]\n";
            });
        return 0;
    }

    /* --stack-file: logic-driven encode/decode through a bound Stack. */
    if (!args.stackFile.empty()) {
        std::unique_ptr<Stack> stack;
        const StackBuilderErr serr = session.loadStack(args.stackFile, stack);
        if (serr.getErrorCode() != StackBuilderErr::STACK_SUCCESS) {
            std::cerr << "Failed to build stack '" << args.stackFile
                      << "': " << serr.getErrorMsg() << "\n";
            return 2;
        }

        const bool jsonOut = (args.output == "json");

        /* Decode path: --hex one frame, or stream from a receiver. */
        if (args.decode) {
            auto decodeAndPrint = [&](const std::vector<uint8_t>& pkt) {
                const Session::StackedPacket d = session.decodeStack(*stack, pkt);
                std::cout << Session::toJson(d) << '\n';
            };

            if (!args.hexInput.empty()) {
                std::vector<uint8_t> raw;
                if (!Session::fromHex(args.hexInput, raw)) {
                    std::cerr << "--hex value is not valid hex\n";
                    return 1;
                }
                decodeAndPrint(raw);
                return 0;
            }

            std::unique_ptr<IReceiver> receiver = makeReceiver(args.transport);
            if (!receiver) {
                std::cerr << "--decode --stack-file requires --transport or --hex\n";
                return 1;
            }
            if (receiver->open().getErrorCode() != ReceiverErr::RECEIVER_SUCCESS) {
                std::cerr << "Failed to open receiver: " << args.transport << "\n";
                return 2;
            }
            size_t n = 0;
            while (args.count == 0 || n < args.count) {
                std::vector<uint8_t> pkt;
                const auto err = receiver->receive(pkt);
                if (err.getErrorCode() == ReceiverErr::RECEIVER_ERR_EOF) break;
                if (err.getErrorCode() != ReceiverErr::RECEIVER_SUCCESS) {
                    std::cerr << "Receive error on packet " << n << "\n";
                    receiver->close();
                    return 2;
                }
                if (!pkt.empty()) { decodeAndPrint(pkt); ++n; }
            }
            receiver->close();
            return 0;
        }

        /* Generate path. */
        std::unique_ptr<ISender> sender = makeSender(args.transport);
        if (sender &&
            sender->open().getErrorCode() != SenderErr::SENDER_SUCCESS) {
            std::cerr << "Failed to open transport: " << args.transport << "\n";
            return 2;
        }
        if (args.seedSet) {
            session.seed(args.seed);
        } else {
            std::random_device rd;
            session.seed(rd());
        }

        for (size_t round = 0; args.count == 0 || round < args.count; ++round) {
            const Session::StackedPacket pkt = session.generateStack(*stack);
            if (jsonOut) std::cout << Session::toJson(pkt) << '\n';
            else         std::cout << Session::toHex(pkt.bytes) << '\n';

            if (sender &&
                sender->send(pkt.bytes).getErrorCode() !=
                    SenderErr::SENDER_SUCCESS) {
                std::cerr << "Send failed on round " << round << "\n";
                sender->close();
                return 2;
            }
        }
        if (sender) sender->close();
        return 0;
    }

    /* Helper: resolve a named interface into a vector, or print error and fail. */
    auto resolveInto = [&](const std::string& name,
                           std::vector<const ProtocolInterface*>& vec) -> bool {
        const ProtocolInterface* p = session.findInterface(name);
        if (!p) {
            std::cerr << "Interface not found: " << name << "\n";
            std::cerr << "Run with --list-interfaces to see available names.\n";
            return false;
        }
        vec.push_back(p);
        return true;
    };

    /* Build the ordered list of interfaces for --connect / --interface. */
    std::vector<const ProtocolInterface*> pipeline;
    /* Build the ordered list of layers for --stack. */
    std::vector<const ProtocolInterface*> stackPipeline;

    if (!args.stack.empty()) {
        for (const auto& name : args.stack)
            if (!resolveInto(name, stackPipeline)) return 2;
    } else if (!args.chain.empty()) {
        for (const auto& name : args.chain)
            if (!resolveInto(name, pipeline)) return 2;
    } else {
        if (!resolveInto(args.iface, pipeline)) return 2;
    }

    const bool jsonOut = (args.output == "json");

    /* --decode: receive packets and decode against the declared interface(s). */
    if (args.decode) {
        auto decodeAndPrint = [&](const std::vector<uint8_t>& pkt) {
            if (!stackPipeline.empty()) {
                const Session::StackedPacket decoded =
                    session.decodeStack(stackPipeline, pkt);
                std::cout << Session::toJson(decoded) << '\n';
            } else {
                const auto result = session.decode(*pipeline[0], pkt);
                if (jsonOut) std::cout << Session::toJson(result) << '\n';
                else         std::cout << Session::toHex(result.bytes) << '\n';
            }
        };

        if (!args.hexInput.empty()) {
            /* Decode a single hex string from the command line. */
            std::vector<uint8_t> raw;
            if (!Session::fromHex(args.hexInput, raw)) {
                std::cerr << "--hex value is not valid hex\n";
                return 1;
            }
            decodeAndPrint(raw);
            return 0;
        }

        /* Stream-based decode from a receiver transport. */
        std::unique_ptr<IReceiver> receiver = makeReceiver(args.transport);
        if (!receiver) {
            std::cerr << "--decode requires --transport <pcap|raw|verilator>:<param>"
                      << " or --hex <hexstring>\n";
            return 1;
        }
        {
            const auto err = receiver->open();
            if (err.getErrorCode() != ReceiverErr::RECEIVER_SUCCESS) {
                std::cerr << "Failed to open receiver: " << args.transport << "\n";
                return 2;
            }
        }

        size_t n = 0;
        while (args.count == 0 || n < args.count) {
            std::vector<uint8_t> pkt;
            const auto err = receiver->receive(pkt);
            if (err.getErrorCode() == ReceiverErr::RECEIVER_ERR_EOF) break;
            if (err.getErrorCode() != ReceiverErr::RECEIVER_SUCCESS) {
                std::cerr << "Receive error on packet " << n << "\n";
                receiver->close();
                return 2;
            }
            if (!pkt.empty()) {
                decodeAndPrint(pkt);
                ++n;
            }
        }
        receiver->close();
        return 0;
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

    if (args.seedSet) {
        session.seed(args.seed);
    } else {
        std::random_device rd;
        session.seed(rd());
    }

    auto sendOrFail = [&](const std::vector<uint8_t>& bytes,
                          size_t round) -> bool {
        if (!sender) return true;
        const auto err = sender->send(bytes);
        if (err.getErrorCode() != SenderErr::SENDER_SUCCESS) {
            std::cerr << "Send failed on round " << round << "\n";
            return false;
        }
        return true;
    };

    /* --stack: generate each layer independently and concatenate bytes.
       count == 0 streams until interrupted, as documented. */
    if (!stackPipeline.empty()) {
        for (size_t round = 0; args.count == 0 || round < args.count; ++round) {
            const Session::StackedPacket pkt =
                session.generateStack(stackPipeline);

            if (jsonOut) std::cout << Session::toJson(pkt) << '\n';
            else         std::cout << Session::toHex(pkt.bytes) << '\n';

            if (!sendOrFail(pkt.bytes, round)) {
                sender->close();
                return 2;
            }
        }
        if (sender) sender->close();
        return 0;
    }

    /* --connect / --interface: one round = one packet per pipeline stage. */
    const bool isChain = pipeline.size() > 1;

    for (size_t round = 0; args.count == 0 || round < args.count; ++round) {
        const std::vector<Session::Layer> stages =
            session.generateChain(pipeline);

        if (isChain && jsonOut) {
            std::cout << Session::toJson(stages) << '\n';
        }

        for (const Session::Layer& stage : stages) {
            const PacketBuilder::BuiltPacket& pkt = stage.second;

            if (!isChain || !jsonOut) {
                if (jsonOut) std::cout << Session::toJson(pkt) << '\n';
                else         std::cout << Session::toHex(pkt.bytes) << '\n';
            }

            if (!sendOrFail(pkt.bytes, round)) {
                sender->close();
                return 2;
            }
        }
    }

    if (sender) sender->close();
    return 0;
}
