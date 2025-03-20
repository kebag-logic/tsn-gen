#pragma once

#include <error.h>
#include <map>
#include <string>

#include <ryml.hpp>

class DatabaseErr: public Error {
public:

    enum {
        DB_SUCCESS,
        DBERR_UNKNOWN,
        DBERR_INVALID_INPUT,
        DBERR_UNEXPECTED,
        DBERR_ENTRY_ALREADY_EXISTS,
        DBERR_ENTRY_NOTFOUND,

        DBERR_ENUM_SENTINEL
    };

    DatabaseErr(void) = delete;
    DatabaseErr(int errorNum): Error(errorNum, mErrVector) {}

    DatabaseErr& operator=(const DatabaseErr other)
    {
        mErrorCode = other.mErrorCode;
        return *this;
    }

private:
    const std::vector<std::string> mErrVector = { 
        "DB:ERR: SUCCESS",
        "DB:ERR: UNKNOWN"
        "DB:ERR: INVALID_INPUT"
        "DB:ERR: UNEXPECTED"
        "DB:ERR: ALREADY EXISTS",
        "DB:ERR: ENTRY NOT FOUND",
        };
};

/**
 * @brief Database that works on the ryml liibrary to add element avoid duplicate,
 *          and notice about such issues.
 */
template <typename T>
class Database  {
public:
    Database();
    /**
     * @brief Add an element to the database based on the NodeRef's string value.
     *          Check if the element is valid, then verify if it exists, then 
     *          adds it.
     * 
     * @return DBERR_INVALID_INPUT when the NodeRef is invalid, then adds the
     *          DBERR_ENTRY_ALREADY_EXISTS when the NodeRef already is in the 
     *          database (shows value vs expected).
     */         
    const DatabaseErr addUniqueElement(ryml::NodeRef& node, T& obj);

private:
    /** 
     * @brief Check if the node ref contains some value, if not it is invalid.
     * @param [in] node: Reference ceon the rapidyaml ref
     * @return DB_SUCCESS if everything is ok, DBERR_INVALID_INPUT if the node
     *          does not contains a value.
     */
    const DatabaseErr checkIfValid(ryml::NodeRef& node);

    /** 
     * @brief Check if the key already exists, 
     * @param [in] node: Reference ceon the rapidyaml ref
     * @return DB_SUCCESS if everything is ok, DBERR_INVALID_INPUT if the node
     *          does not contains a value.
     */
    const DatabaseErr checkIfExists(ryml::NodeRef& node);

    /**
     * The database object = InternalMaps[string&]
     */
    std::map<std::reference_wrapper<std::string>, T> internalMaps;

};