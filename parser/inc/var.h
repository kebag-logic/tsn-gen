/*
 * SPDX-FileCopyrightText: 2025 Kebag-Logic (https://kebag-logic.com)
 * SPDX-FileCopyrightText: 2025 Alexandre Malki <alexandre.malki@kebag-logic.com>
 * SPDX-License-Identifier: MIT
 */


 #pragma once
#include <cstdint>
#include <optional>
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

    /**
     * @brief Inclusive integer range constraint [min, max].
     *
     * When present, the PacketBuilder draws values uniformly from this
     * range instead of from the full [0, 2^size−1] space.
     */
    struct Range {
        int32_t min;
        int32_t max;
    };

    /**
     * @param name           Fully qualified name (service::var_name).
     * @param size           Field width in bits.
     * @param expectedValues Exhaustive list of allowed values (empty = unconstrained).
     * @param range          Optional [min, max] range; ignored when expectedValues is set.
     * @param mask           Bitmask: only bits within the mask may be set (0 = unconstrained).
     */
    Var(const std::string& name, uint32_t size = 0,
        std::vector<int32_t> expectedValues = {},
        std::optional<Range> range = std::nullopt,
        uint64_t mask = 0)
        : mName(name), mSize(size),
          mExpectedValues(std::move(expectedValues)),
          mRange(range),
          mMask(mask) {}
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

    /** True when a [min, max] range constraint is defined. */
    bool hasRange() const { return mRange.has_value(); }
    const Range& getRange() const { return *mRange; }

    /** True when a bitmask constraint is defined (mask != 0). */
    bool hasMask() const { return mMask != 0; }
    uint64_t getMask() const { return mMask; }

protected:
    const std::string mName;
    const uint32_t mSize;
    const std::vector<int32_t> mExpectedValues;
    const std::optional<Range> mRange;
    const uint64_t mMask;
};
