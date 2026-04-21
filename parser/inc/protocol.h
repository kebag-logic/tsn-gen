/*
 * SPDX-FileCopyrightText: 2025 Kebag-Logic (https://kebag-logic.com)
 * SPDX-FileCopyrightText: 2025 Alexandre Malki <alexandre.malki@kebag-logic.com>
 * SPDX-License-Identifier: MIT
 */


 #pragma once

#include <functional>
#include <memory>
#include <string>
#include <vector>
#include <map>

#include <database.h>
#include <error.h>
#include <entity.h>
#include <protocol_interface.h>
#include <service.h>
#include <var.h>

#include <ryml.hpp>

class ProtocolErr: public Error {
public:
    enum {
        PROTO_SUCCESS,
        PROTOERR_UNKNOWN,
        PROTOERR_INVALID_FILE,
        PROTOERR_UNEXPECTED,

        PROTOERR_ENUM_SENTINEL
    };

    ProtocolErr(void) = delete;
    ProtocolErr(int errorNum): Error(errorNum, mErrVector) {}

    ProtocolErr& operator=(const ProtocolErr other)
    {
        mErrorCode = other.mErrorCode;
        return *this;
    }

private:
    const std::vector<std::string> mErrVector = {
        "PPOTO:ERR: SUCCESS",
        "PROTO:ERR: UNKNOWN",
        "PROTO:ERR: INVALID FILE",
        "PROTO:ERR: UNEXPECTED"
    };
};

class Protocol {
public:
	/**
	 * The path is usually the same as the
	 * Namespace application/17221_.../name
	 */
	Protocol(const std::string& path);
    Protocol(void) = delete;

    /**
     * @brief Validates the protocol file without updating any database.
     */
    const ProtocolErr parseProtocolFile(void);

    /**
     * @brief Parses the protocol file and populates the provided databases
     *        with vars and interfaces found in the file.
     * @param [in, out] dbVars      global variable database
     * @param [in, out] dbIfproto   global interface database
     */
    const ProtocolErr parseProtocolFile(
        Database<Var>& dbVars, Database<ProtocolInterface>& dbIfproto);

    /**
     * @brief Parses the protocol file and also records per-service metadata
     *        (name + optional `logic:`) into @p dbServices. Vars and
     *        interfaces go into @p dbVars / @p dbIfproto as usual.
     */
    const ProtocolErr parseProtocolFile(
        Database<Var>& dbVars, Database<ProtocolInterface>& dbIfproto,
        Database<ProtocolService>& dbServices);

private:
    /** @brief Check a node against expected type and number format.
     *  @param node    node to be checked
     *  @param parser  parser used to retrieve error location
     *  @param type    expected node type bits
     *  @param checkisNumber  whether the value must be a number
     */
    const ProtocolErr checkNodeValidity(ryml::ConstNodeRef& node,
                                        std::unique_ptr<ryml::Parser>& parser,
                                        ryml::NodeType_e type, bool checkisNumber);

    /**
     * @brief Parses the variables defined in the protocol file and inserts
     *        them into dbVars using a fully qualified name.
     */
    const ProtocolErr getParsedProtocolVars(Database<Var>& dbVars);

    /**
     * @brief Parses the entities/interfaces section and inserts each interface
     *        into dbIfProto using a fully qualified name.
     */
    const ProtocolErr getParsedProtocolInterface(
                                    Database<ProtocolInterface>& dbIfProto);

    /**
     * @brief Registers the service under its name, carrying the optional
     *        `logic:` module name if present in the YAML.
     */
    const ProtocolErr getParsedProtocolService(
                                    Database<ProtocolService>& dbServices);

    /**
     * @brief Parses the service name node.
     * @param [in]     parser     parser used to retrieve error location
     * @param [in,out] protoName  set to the service name node on success
     * @return PROTOERR_INVALID_FILE if the node is missing or malformed,
     *         PROTO_SUCCESS otherwise.
     */
    const ProtocolErr getProtocolName(std::unique_ptr<ryml::Parser>& parser,
                                      ryml::ConstNodeRef& protoName);

    /** Path of the YAML file being parsed. */
    std::string mPath;

    /** The service name node (references mTree). */
    ryml::ConstNodeRef mProtocolName;

    std::map<std::string, Entity> mEntities;
    std::map<std::string, Var> mVar;

    /**
     * Owns the parsed YAML data; must outlive all ConstNodeRef members.
     */
    ryml::Tree mTree;
};
