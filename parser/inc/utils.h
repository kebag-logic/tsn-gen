
#pragma once
#include <unistd.h>

/**
 * @brief A scopped file descriptor to close it when we exists, no need
 *          for more in the code
 */
class ScopedFD {
public:
    ScopedFD(int fd) : mFD(fd) {}
    ScopedFD() = delete;

        
    /**
     * @brief overload the Comparision whether is smaller or greater is here
     */
    ScopedFD& operator=(ScopedFD& other)
    {
        mFD = other.mFD;
        return *this;
    }
    
    /**
     * @brief overload the Comparision whether is smaller or greater is here
     */
    bool operator<(int value) 
    {
        return (mFD < value);
    }

    /**
     * @brief  The Filedescriptor is an integer, so return an int when needed
     */
    operator int () const
    {
        return mFD;
    }

    ~ScopedFD()
    {
        close(mFD);
    }

private:
    int mFD;
};