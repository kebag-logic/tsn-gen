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

const ProtocolErr Protocol::getParsedProtocolName(void)
{
    ProtocolErr err(ProtocolErr::PROTOERR_UNKNOWN);

    
    mProtocolName = mTree["service"];
    if (mProtocolName.invalid()) {
        std::cerr << "ERR: missing service node in " << mPath << std::endl;
        err.setErrorCode(ProtocolErr::PROTOERR_INVALID_FILE);
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
    opts.locations(true); // enable locations, default is false
    ryml::EventHandlerTree evt_handler = {};
    ryml::Parser parser(&evt_handler, opts);

    mTree = ryml::parse_in_arena(&parser, ryml::csubstr(mPath.c_str()),
                 ryml::csubstr(static_cast<char *>(ptr)));
    close(fd);

    /** 
     * First parse the protocol name 
     */
    err = getParsedProtocolName();
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