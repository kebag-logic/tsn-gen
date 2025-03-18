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