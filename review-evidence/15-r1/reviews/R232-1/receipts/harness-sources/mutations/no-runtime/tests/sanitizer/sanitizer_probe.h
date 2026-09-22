/*
 * SPDX-FileCopyrightText: 2026 Kebag-Logic (https://kebag-logic.com)
 * SPDX-FileCopyrightText: 2026 Alexandre Malki <alexandre.malki@kebag-logic.com>
 * SPDX-License-Identifier: MIT
 *
 * Fixture library of the sanitizer helper check. generate_test_libs builds
 * it, so its _asan/_ubsan variants carry exactly the helpers' instrumentation.
 */

#pragma once

#include <cstddef>

namespace sanitizer_probe {

/* values[index]: out of bounds when index is past the allocation. */
int libraryRead(const int* values, std::size_t index);

/* lhs + rhs in int: undefined when the sum leaves int's range. */
int libraryAdd(int lhs, int rhs);

} // namespace sanitizer_probe
