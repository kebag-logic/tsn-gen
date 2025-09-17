/*
 * SPDX-FileCopyrightText: 2025 Kebag-Logic (https://kebag-logic.com)
 * SPDX-FileCopyrightText: 2025 Alexandre Malki <alexandre.malki@kebag-logic.com>
 * SPDX-License-Identifier: MIT
 */
 

 #pragma once

#include <cstdint>

template<typename T>
class VarType {
public:
	VarType();
    
    int setVar(T type) {
        mValue = type;
        return 0;
    };

    int setSize(ssize_t size)
    {
        mSize = size;
        return 0;
    }
private:
    ssize_t mSize;
    T mValue;
};
