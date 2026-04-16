/*
 * SPDX-FileCopyrightText: 2025 Kebag-Logic (https://kebag-logic.com)
 * SPDX-FileCopyrightText: 2025 Alexandre Malki <alexandre.malki@kebag-logic.com>
 * SPDX-License-Identifier: MIT
 */


 #pragma once

#include <error.h>
#include <map>
#include <string>

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
        "DB:ERR: UNKNOWN",
        "DB:ERR: INVALID_INPUT",
        "DB:ERR: UNEXPECTED",
        "DB:ERR: ALREADY EXISTS",
        "DB:ERR: ENTRY NOT FOUND",
        };
};

/**
 * @brief Database that stores named elements, prevents duplicates,
 *        and provides lookup by name.
 */
template <typename T>
class Database  {
public:
    Database();

    /**
     * @brief Add an element to the database by name.
     *          Checks if name is valid (non-empty), then verifies it does not
     *          already exist, then inserts the element.
     *
     * @return DBERR_INVALID_INPUT when the name is empty.
     *         DBERR_ENTRY_ALREADY_EXISTS when the name is already in the database.
     *         DB_SUCCESS on success.
     */
    const DatabaseErr addUniqueElement(const std::string& name, const T& obj);

    /**
     * @brief Retrieve a pointer to an element by name. Returns nullptr if not found.
     */
    const T* getElement(const std::string& name) const;

    /**
     * @brief Return the number of stored elements.
     */
    size_t size() const;

private:
    /**
     * @brief Check if the name is non-empty.
     */
    const DatabaseErr checkIfValid(const std::string& name) const;

    /**
     * @brief Check if the name already exists in the database.
     */
    const DatabaseErr checkIfExists(const std::string& name) const;

    std::map<std::string, T> internalMaps;
};
