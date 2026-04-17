/*
 * SPDX-FileCopyrightText: 2025 Kebag-Logic (https://kebag-logic.com)
 * SPDX-FileCopyrightText: 2025 Alexandre Malki <alexandre.malki@kebag-logic.com>
 * SPDX-License-Identifier: MIT
 */


 #include <protocol.h>
#include <utils.h>

#include <cstdint>
#include <iostream>
#include <string>

#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>


Protocol::Protocol(const std::string& path) :
    mPath(path),
    mProtocolName(),
    mTree(ryml::Tree())
{
}

const ProtocolErr Protocol::checkNodeValidity(ryml::ConstNodeRef &node,
                                    std::unique_ptr<ryml::Parser> &parser,
                                    ryml::NodeType_e type, bool checkisNumber)
{
    ProtocolErr err(ProtocolErr::PROTOERR_UNKNOWN);

    if (node.id() == ryml::NONE) {
        err.setmExtMsg("Node does not exists");
        err.setErrorCode(ProtocolErr::PROTOERR_INVALID_FILE);
    } else if ( node.key_is_null() || node.invalid()) {
        err.setmExtMsg("Key is null and Invalid");
        err.setErrorCode(ProtocolErr::PROTOERR_INVALID_FILE);
    } else {
        std::cout << "INF: " << "Parsing protocol " << std::endl;
        if (!node.type_has_any(type)) {
            err.setmExtMsg("un-expected type");
            err.setErrorCode(ProtocolErr::PROTOERR_UNEXPECTED);
        } else if (node.val().is_number() != checkisNumber) {
            err.setErrorCode(ProtocolErr::PROTOERR_UNEXPECTED);
            err.setmExtMsg("No number expected");
        } else {
            err.setErrorCode(ProtocolErr::PROTO_SUCCESS);
        }

        if (err.getErrorCode()) {
            ryml::Location loc = parser->location(node);
            std::cerr << "ERR: YAML " << node.key() << " L:" << loc.line <<
                        " C:" << loc.col << std::endl;

            std::cerr << "     " << node.key() <<
                    err.getmExtMsg() << std::endl;
        }
    }

    return err;
}

// TODO decouple this
const ProtocolErr Protocol::getProtocolName(std::unique_ptr<ryml::Parser>& parser,
                                            ryml::ConstNodeRef& protoName)
{
    ProtocolErr err(ProtocolErr::PROTOERR_UNKNOWN);
    const std::string nodeName = "service";

    ryml::ConstNodeRef serviceNode = mTree[nodeName.c_str()];

    err = checkNodeValidity(serviceNode, parser, ryml::NodeType_e::VAL,
                        false);
    if  (!err.getErrorCode()) {
        protoName = serviceNode;
    }

    return err;
}

static std::string csubstrToString(ryml::csubstr s)
{
    return std::string(s.str, s.len);
}

const ProtocolErr Protocol::getParsedProtocolVars(Database<Var>& dbVars)
{
    ProtocolErr err(ProtocolErr::PROTO_SUCCESS);

    std::string serviceName = csubstrToString(mProtocolName.val());

    ryml::ConstNodeRef varsNode = mTree["vars"];
    if (varsNode.invalid() || !varsNode.is_seq()) {
        return err;
    }

    for (ryml::ConstNodeRef varItem : varsNode) {
        if (!varItem.has_child("var")) {
            continue;
        }

        std::string varName = csubstrToString(varItem["var"].val());
        std::string qualifiedName = serviceName + "::" + varName;

        uint32_t size = 0;
        if (varItem.has_child("size")) {
            std::string sizeStr = csubstrToString(varItem["size"].val());
            size = static_cast<uint32_t>(std::stoul(sizeStr));
        }

        std::vector<int32_t> expectedValues;
        std::optional<Var::Range> range;
        uint64_t mask = 0;

        if (varItem.has_child("expected")) {
            ryml::ConstNodeRef expectedNode = varItem["expected"];

            /* expected.values: [v0, v1, ...] — exhaustive allowed list */
            if (expectedNode.has_child("values")) {
                ryml::ConstNodeRef valuesNode = expectedNode["values"];
                if (valuesNode.is_seq()) {
                    for (ryml::ConstNodeRef valNode : valuesNode) {
                        std::string valStr = csubstrToString(valNode.val());
                        expectedValues.push_back(
                            static_cast<int32_t>(
                                std::stol(valStr, nullptr, 0)));
                    }
                }
            }

            /* expected.value: N — single fixed value */
            if (expectedNode.has_child("value")) {
                std::string valStr =
                    csubstrToString(expectedNode["value"].val());
                expectedValues.push_back(
                    static_cast<int32_t>(std::stol(valStr, nullptr, 0)));
            }

            /* expected.range: [min, max] — inclusive integer range */
            if (expectedNode.has_child("range")) {
                ryml::ConstNodeRef rangeNode = expectedNode["range"];
                if (rangeNode.is_seq() && rangeNode.num_children() == 2) {
                    std::vector<int32_t> rvals;
                    for (ryml::ConstNodeRef rv : rangeNode) {
                        rvals.push_back(static_cast<int32_t>(
                            std::stol(csubstrToString(rv.val()), nullptr, 0)));
                    }
                    range = Var::Range{rvals[0], rvals[1]};
                }
            }

            /* expected.mask: [M] — only bits within M may be set */
            if (expectedNode.has_child("mask")) {
                ryml::ConstNodeRef maskNode = expectedNode["mask"];
                if (maskNode.is_seq()) {
                    for (ryml::ConstNodeRef mv : maskNode) {
                        mask = static_cast<uint64_t>(std::stoull(
                            csubstrToString(mv.val()), nullptr, 0));
                        break; /* take first entry only */
                    }
                }
            }
        }

        Var var(qualifiedName, size, std::move(expectedValues), range, mask);
        DatabaseErr dbErr = dbVars.addUniqueElement(qualifiedName, var);
        if (dbErr.getErrorCode() == DatabaseErr::DBERR_ENTRY_ALREADY_EXISTS) {
            std::cerr << "WARN: var '" << qualifiedName
                      << "' already in database, skipping\n";
        }
    }

    return err;
}

const ProtocolErr Protocol::getParsedProtocolInterface(
    Database<ProtocolInterface>& dbIfProto)
{
    ProtocolErr err(ProtocolErr::PROTO_SUCCESS);

    std::string serviceName = csubstrToString(mProtocolName.val());

    ryml::ConstNodeRef entitiesNode = mTree["entities"];
    if (entitiesNode.invalid() || !entitiesNode.is_seq()) {
        return err;
    }

    for (ryml::ConstNodeRef entityItem : entitiesNode) {
        if (!entityItem.has_child("entity")) {
            continue;
        }

        std::string entityName = csubstrToString(entityItem["entity"].val());

        if (!entityItem.has_child("interfaces")) {
            continue;
        }

        ryml::ConstNodeRef interfacesNode = entityItem["interfaces"];
        if (!interfacesNode.is_seq()) {
            continue;
        }

        for (ryml::ConstNodeRef ifItem : interfacesNode) {
            if (!ifItem.is_map() || !ifItem.has_child("interface")) {
                continue;
            }

            std::string ifName = csubstrToString(ifItem["interface"].val());

            ProtocolInterface::IfDirections dir = ProtocolInterface::IN;
            if (ifItem.has_child("dir")) {
                std::string dirStr = csubstrToString(ifItem["dir"].val());
                if (dirStr == "out") {
                    dir = ProtocolInterface::OUT;
                }
            }

            std::vector<std::string> varRefs;
            if (ifItem.has_child("vars")) {
                ryml::ConstNodeRef varsRefNode = ifItem["vars"];
                if (varsRefNode.is_map()) {
                    /* map form: vars: { var_ref: name, ... } */
                    for (ryml::ConstNodeRef refItem : varsRefNode) {
                        std::string refVal = csubstrToString(refItem.val());
                        varRefs.push_back(serviceName + "::" + refVal);
                    }
                } else if (varsRefNode.is_seq()) {
                    /* sequence form: vars: [{ var_ref: name }, ...] */
                    for (ryml::ConstNodeRef refItem : varsRefNode) {
                        if (!refItem.is_map() || !refItem.has_child("var_ref")) {
                            continue;
                        }
                        std::string refVal =
                            csubstrToString(refItem["var_ref"].val());
                        /* already qualified (contains "::") → keep as-is */
                        if (refVal.find("::") != std::string::npos) {
                            varRefs.push_back(refVal);
                        } else {
                            varRefs.push_back(serviceName + "::" + refVal);
                        }
                    }
                }
            }

            std::string qualifiedName =
                serviceName + "::" + entityName + "::" + ifName;

            ProtocolInterface iface(qualifiedName, dir, std::move(varRefs));
            DatabaseErr dbErr = dbIfProto.addUniqueElement(qualifiedName, iface);
            if (dbErr.getErrorCode() == DatabaseErr::DBERR_ENTRY_ALREADY_EXISTS) {
                std::cerr << "WARN: interface '" << qualifiedName
                          << "' already in database, skipping\n";
            }
        }
    }

    return err;
}

const ProtocolErr Protocol::parseProtocolFile(void)
{
    Database<Var> tmpVars;
    Database<ProtocolInterface> tmpIfproto;
    return parseProtocolFile(tmpVars, tmpIfproto);
}

const ProtocolErr Protocol::parseProtocolFile(
        Database<Var>& dbVars, Database<ProtocolInterface>& dbIfproto)
{
    ProtocolErr err(ProtocolErr::PROTO_SUCCESS);
    void* ptr;

    ScopedFD fd (open(mPath.c_str(), O_RDONLY));
    if (fd < 0) {
        std::cerr << "ERR: Could not open file " << mPath << std::endl;
        return err;
    }

    off_t len = lseek(fd, 0, SEEK_END);
    if (len < 0) {
        std::cerr << "ERR: lseek for the end of the file " << mPath <<
                     " of size " << (uint64_t)len << std::endl;
        return err;
    }

    ptr = (char *) mmap(0, len, PROT_READ|PROT_WRITE, MAP_PRIVATE, fd, 0);
    if (ptr == NULL) {
        std::cerr << "ERR: mmap fd=" << fd << " for file " << mPath
                    << " of size " << len << std::endl;
        return err;
    }

    ryml::ParserOptions opts = {};
    opts.locations(true);
    ryml::EventHandlerTree evt_handler = {};
    std::unique_ptr<ryml::Parser> parser =
                  std::make_unique<ryml::Parser>(&evt_handler, opts);

    mTree = ryml::parse_in_arena(parser.get(), ryml::csubstr(mPath.c_str()),
                 ryml::csubstr(static_cast<char *>(ptr)));

    munmap(ptr, len);

    if (mTree.empty() || !mTree.rootref().is_map()) {
        err.setErrorCode(ProtocolErr::PROTOERR_INVALID_FILE);
        std::cerr << "Invalid file (empty or not a YAML map): " <<  mPath << std::endl;
        return err;
    }

    err = getProtocolName(parser, mProtocolName);
    if (err.getErrorCode()) {
        return err;
    }

    err = getParsedProtocolVars(dbVars);
    if (err.getErrorCode()) {
        return err;
    }

    err = getParsedProtocolInterface(dbIfproto);
    if (err.getErrorCode()) {
        return err;
    }

    return err;
}
