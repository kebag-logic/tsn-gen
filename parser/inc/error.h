/*
 * SPDX-FileCopyrightText: 2025 Kebag-Logic (https://kebag-logic.com)
 * SPDX-FileCopyrightText: 2025 Alexandre Malki <alexandre.malki@kebag-logic.com>
 * SPDX-License-Identifier: MIT
 */
 

 #pragma once

#include <cassert>
#include <string>
#include <vector>


class Error {
public:
    Error() = delete;

    Error(int ErrorCode, const std::vector<std::string>& errorVector):
        mErrorCode(ErrorCode), mErrorVector(errorVector)
    {
    }

    /**
     * @brief retrive the error code
     */
    int getErrorCode(void) const
    {
        return mErrorCode;
    }

    /***
     * @brief set the error code of the according the max value possible
     */
    void setErrorCode(const int errorCode) {
        assert((void("ERR: Invalid mErrorCode"), mErrorCode < mErrorVector.size()));
        mErrorCode = errorCode;
    }

    /**
     * @brief retrieve the String message link with the
     * error message
     *
     * @return the message
     */
    const std::string& getErrorMsg(void) const
    {
        assert((void("ERR: Invalid mErrorCode"), mErrorCode < mErrorVector.size()));
        return mErrorVector[mErrorCode];
    }

    /**
     * @brief Setting the external message, can be re-used later
     */
    void setmExtMsg(const std::string& msg)
    {
        mExtMsg = msg;
    }

    /**
     * @brief retrieved the setted external message
     */
    const std::string& getmExtMsg(void) const
    {
        return mExtMsg;
    }

protected:
    int mErrorCode;
    const std::vector<std::string>& mErrorVector;
    std::string mExtMsg;
};
