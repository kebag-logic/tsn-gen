/*
 * SPDX-FileCopyrightText: 2026 Kebag-Logic (https://kebag-logic.com)
 * SPDX-FileCopyrightText: 2026 Alexandre Malki <alexandre.malki@kebag-logic.com>
 * SPDX-License-Identifier: MIT
 *
 * Probe of the sanitizer helpers. target_link_testlibs builds this TU as the
 * plain, _ASAN and _UBSAN executables, the sanitized ones taking their
 * instrumentation from the fixture variant's PUBLIC compile options, as every
 * component test does.
 *
 * Each faulting mode puts its defect in exactly one TU: the fixture library
 * (library-*) or this consuming TU (consumer-*). Only compiler
 * instrumentation of that TU can report it; linking the sanitizer runtime
 * alone cannot. Every mode prints "started" before its operation and
 * "completed" after it, so a run that went on past a fault is visible.
 */

#include "sanitizer_probe.h"

#include <cstdio>
#include <cstring>
#include <limits>
#include <memory>

namespace {

int consumerRead(const int* values, std::size_t index)
{
    return values[index];
}

int consumerAdd(int lhs, int rhs)
{
    return lhs + rhs;
}

} // namespace

int main(int argc, char** argv)
{
    if (argc != 2) {
        std::fprintf(stderr, "usage: %s clean|library-heap-overflow|"
                             "consumer-heap-overflow|library-signed-overflow|"
                             "consumer-signed-overflow\n", argv[0]);
        return 2;
    }
    const char* mode = argv[1];

    /* volatile, so no optimisation level can fold the index or operands. */
    volatile std::size_t count = 4;
    volatile int maxInt = std::numeric_limits<int>::max();
    volatile int one = 1;
    std::unique_ptr<int[]> values(new int[count]());

    std::printf("sanitizer_probe: %s started\n", mode);
    std::fflush(stdout);

    int value = 0;
    if (std::strcmp(mode, "clean") == 0) {
        value = sanitizer_probe::libraryRead(values.get(), count - 1) +
                consumerRead(values.get(), count - 1) +
                sanitizer_probe::libraryAdd(one, one) + consumerAdd(one, one);
    } else if (std::strcmp(mode, "library-heap-overflow") == 0) {
        value = sanitizer_probe::libraryRead(values.get(), count);
    } else if (std::strcmp(mode, "consumer-heap-overflow") == 0) {
        value = consumerRead(values.get(), count);
    } else if (std::strcmp(mode, "library-signed-overflow") == 0) {
        value = sanitizer_probe::libraryAdd(maxInt, one);
    } else if (std::strcmp(mode, "consumer-signed-overflow") == 0) {
        value = consumerAdd(maxInt, one);
    } else {
        std::fprintf(stderr, "unknown mode: %s\n", mode);
        return 2;
    }

    std::printf("sanitizer_probe: %s completed (value %d)\n", mode, value);
    return 0;
}
