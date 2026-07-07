/*
 * SPDX-FileCopyrightText: 2026 Kebag-Logic (https://kebag-logic.com)
 * SPDX-FileCopyrightText: 2026 Alexandre Malki <alexandre.malki@kebag-logic.com>
 * SPDX-License-Identifier: MIT
 */

#include <tsn/log.h>

#include <iostream>
#include <streambuf>

namespace {

/* Discards every character; backs the stream returned for suppressed
 * levels so call sites never need an "is enabled" branch. */
class NullBuffer final : public std::streambuf {
protected:
    int overflow(int c) override { return c; }
};

NullBuffer gNullBuffer;
std::ostream gNullStream(&gNullBuffer);

tsn::LogLevel gLevel = tsn::LogLevel::warn;

} /* anonymous namespace */

namespace tsn {

LogLevel logLevel()
{
    return gLevel;
}

void setLogLevel(LogLevel lvl)
{
    gLevel = lvl;
}

bool setLogLevel(const std::string& name)
{
    if (name == "quiet")      { gLevel = LogLevel::quiet; }
    else if (name == "error") { gLevel = LogLevel::error; }
    else if (name == "warn")  { gLevel = LogLevel::warn;  }
    else if (name == "info")  { gLevel = LogLevel::info;  }
    else if (name == "debug") { gLevel = LogLevel::debug; }
    else                      { return false; }
    return true;
}

std::ostream& log(LogLevel lvl)
{
    if (lvl == LogLevel::quiet ||
        static_cast<int>(lvl) > static_cast<int>(gLevel)) {
        return gNullStream;
    }
    return std::cerr;
}

} /* namespace tsn */
