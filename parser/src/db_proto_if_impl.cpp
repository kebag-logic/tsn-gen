/*
 * SPDX-FileCopyrightText: 2025 Kebag-Logic (https://kebag-logic.com)
 * SPDX-FileCopyrightText: 2025 Alexandre Malki <alexandre.malki@kebag-logic.com>
 * SPDX-License-Identifier: MIT
 */

/*
 * Explicit template instantiation for Database<ProtocolInterface>.
 * Kept separate so the translation unit can be compiled with different
 * sanitiser flags without re-instantiating every database specialisation.
 */

#include <tsn/protocol_interface.h>
#include <tsn/database.cpp>

namespace tsn {

template class Database<ProtocolInterface>;

} /* namespace tsn */
