/*
 * SPDX-FileCopyrightText: 2026 Kebag-Logic (https://kebag-logic.com)
 * SPDX-FileCopyrightText: 2026 Alexandre Malki <alexandre.malki@kebag-logic.com>
 * SPDX-License-Identifier: MIT
 */

#include "sanitizer_probe.h"

namespace sanitizer_probe {

int libraryRead(const int* values, std::size_t index)
{
    return values[index];
}

int libraryAdd(int lhs, int rhs)
{
    return lhs + rhs;
}

} // namespace sanitizer_probe
