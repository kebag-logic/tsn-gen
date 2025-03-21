#include <protocol.h>
#include <utils.h>

#include <cstdint>
#include <iostream>

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
        /** Check if the node exists and refactor */
        err.setmExtMsg("Node does not exists");
        err.setErrorCode(ProtocolErr::PROTOERR_INVALID_FILE);
    } else if ( node.key_is_null() || node.invalid()) {
        /** Check if the node exists and refactor */
        err.setmExtMsg("Key is null and Invalid");
        err.setErrorCode(ProtocolErr::PROTOERR_INVALID_FILE);
    } else {
        /** Checking whether the node is the one we want to have */
        std::cout << "INF: " << "Parsing protocol " << std::endl;
        if (!(node.id() & type)) {
            err.setmExtMsg("un-expected type");
            err.setErrorCode(ProtocolErr::PROTOERR_UNEXPECTED);
        } else if (node.val().is_number() != checkisNumber) {
            /** Fr */
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
        /** No error so this is our node */
       protoName = serviceNode;
    }

    return err;
}

const ProtocolErr Protocol::getParsedProtocolInterface(
    Database<ProtocolInterface>& dbIfProto
)
{
    ProtocolErr err(ProtocolErr::PROTO_SUCCESS);

    return err;
}

const ProtocolErr Protocol::getParsedProtocolVars(Database<Var>& dbVars)
{
    ProtocolErr err(ProtocolErr::PROTO_SUCCESS);

    return err;
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

    if (mTree.empty()) {
        err.setErrorCode(ProtocolErr::PROTOERR_INVALID_FILE);
        std::cerr << "Invalid file: " <<  mPath << std::endl;
        return err;
    }

    /**
     * First parse the protocol name
     */
    err = getProtocolName(parser, mProtocolName);
    if (err.getErrorCode()) {
        return err;
    }

    /**
     * Then parse the vars
     */
    // err = getParsedProtocolVars(dbVars);
    // if (err.getErrorCode()) {
    //     return err;
    // }

    /**
     * Then parse the vars
     */
   // err = getParsedProtocolInterface();
    // if (err.getErrorCode()) {
    //     return err;
    // }
    return err;
}