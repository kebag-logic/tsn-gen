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
        "PROTO:ERR: UNKNOWN"
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
    Protocol(void) = delete ;

    /**
     * @brief Parsed the basis for the protocol/service that is find.
     */
    const ProtocolErr parseProtocolFile(void);

    /**
     * @brief Parsed of the interface, will add Vars and interfaces
     * @param [in, out] dbVars variables
     * @param [in, out] dbIfProto interfaces
     */
    const ProtocolErr parseProtocolFile(
        Database<Var>& dbVars, Database<ProtocolInterface>& dbIfproto);

private:
    /** @brief Checking nodes with expeted type and format
     *  @param node, node to be checked
     *
    */
    const ProtocolErr checkNodeValidity(ryml::ConstNodeRef& node,
                                        std::unique_ptr<ryml::Parser>& parser,
                                        ryml::NodeType_e type, bool checkisNumber);
    /**
     * @brief Parses the variable defined in the protocol,
     *      If there are duplicate, they will be checked.
     */
    const ProtocolErr getParsedProtocolVars(Database<Var>& dbVars);

    /**
     * @brief Parses the interface, and feed it in the global inteface.
     *          Duplicate will not be added.
     * @return If a same interface exists DBERR_ENTRY_ALREADY_EXISTS
     */
    const ProtocolErr getParsedProtocolInterface(
                                    Database<ProtocolInterface>& dbIfProto);
    /**
     * @brief Parses the variable defined in the protocol
     */
    const ProtocolErr getParsedProtocolVars(
                                        Database<ProtocolInterface>& dbIfProto);

    /**
     * @brief Checking for the name of the function
     * @param [in] parser, the parser used to retrieve the location of the node
     * @param [in, out] protoName: retrieved parsed protocol name
     * @return PROTOERR_INVALID_FILE if the necessary node are not found
     *          otherwise it will return PROTO_SUCCESS
     */
    const ProtocolErr getProtocolName(std::unique_ptr<ryml::Parser> &parser,
                                      ryml::ConstNodeRef &protoName);

        /**
         * This is the path of the YAML file being parsed.
         */
        std::string mPath;

    /*
     * This is the Name of the protocol
     */
	ryml::ConstNodeRef mProtocolName;

    std::map<std::string, Entity> mEntities;
	std::map<std::reference_wrapper<std::string>, Var&> mVar;


    /**
     * Use the mTree yaml for now as a reference, once allocated we do not
     * need the file anymore
     */
    ryml::Tree mTree;
};
