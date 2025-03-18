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

    int getErrorCode(void) const {
        return mErrorCode;
    }

    void setErrorCode(const int errorCode) {
        assert((void("ERR: Invalid mErrorCode"), mErrorCode < mErrorVector.size()));
        mErrorCode = errorCode;
    }

    const std::string& getErrorMsg(void) {
        assert((void("ERR: Invalid mErrorCode"), mErrorCode < mErrorVector.size()));
        return mErrorVector[mErrorCode];
    }

protected:
    int mErrorCode;
    const std::vector<std::string>& mErrorVector;
};