/*
 * SPDX-FileCopyrightText: 2025 Kebag-Logic (https://kebag-logic.com)
 * SPDX-FileCopyrightText: 2025 Alexandre Malki <alexandre.malki@kebag-logic.com>
 * SPDX-License-Identifier: MIT
 */


 #pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <error.h>

class VarErr: public Error {
public:

    enum {
        VAR_SUCCESS,
        VAR_ERR_UNKNOWN,
        VAR_ERR_UNVALID_VAR,
        VAR_ENUM_SENTINEL
    };

    VarErr( ) : Error(VAR_ERR_UNKNOWN, mErrorVector)  {}
    VarErr(int errorNum): Error(errorNum, mErrorVector) {}

private:
    const std::vector<std::string> mErrorVector = {
        "VAR: ERR: SUCCESS",
        "VAR: ERR: UNKNOWN",
        "VAR: ERR: INVALID NAME",
    };

};

class Var {
public:
    /** This is the name of the var with the whole path so for the case of adp,
     *  located in protocol/application/1722_1/adp/1722_1_adp.yaml it shall be named:
     *
     *  protocol::application::1722_1::adp::1722_1_adp::var::NAMEOFVAR
     */
    Var(const std::string& name, uint32_t size = 0,
        std::vector<int32_t> expectedValues = {})
        : mName(name), mSize(size), mExpectedValues(std::move(expectedValues)) {}
    Var() = delete;

    const VarErr getVarName(std::string& name) const
    {
        VarErr err(VarErr::VAR_SUCCESS);

        if (!mName.length()) {
            err.setErrorCode(VarErr::VAR_ERR_UNVALID_VAR);
        } else {
            name = mName;
        }
        return err;
    }

    uint32_t getSize() const { return mSize; }

    const std::vector<int32_t>& getExpectedValues() const { return mExpectedValues; }

protected:
    const std::string mName;
    const uint32_t mSize;
    const std::vector<int32_t> mExpectedValues;
};
