/*
 * SPDX-FileCopyrightText: 2025 Kebag-Logic (https://kebag-logic.com)
 * SPDX-FileCopyrightText: 2025 Alexandre Malki <alexandre.malki@kebag-logic.com>
 * SPDX-License-Identifier: MIT
 */

/*
 * Placeholder translation unit.
 * Database<Protocol> is intentionally not explicitly instantiated here
 * because Protocol owns a ryml::Tree and is not used as a database element.
 * The explicit instantiation for Database<ProtocolInterface> lives in
 * db_proto_if_impl.cpp.
 */

#include <protocol.h>
#include <database.h>
