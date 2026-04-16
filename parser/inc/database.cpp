/*
 * SPDX-FileCopyrightText: 2025 Kebag-Logic (https://kebag-logic.com)
 * SPDX-FileCopyrightText: 2025 Alexandre Malki <alexandre.malki@kebag-logic.com>
 * SPDX-License-Identifier: MIT
 */

/*
 * Template implementation file — not compiled directly.
 * Included by explicit instantiation files (db_*_impl.cpp).
 */

#include <database.h>
#include <iostream>

template <typename T>
Database<T>::Database() {}

template <typename T>
const DatabaseErr Database<T>::checkIfValid(const std::string& name) const
{
    DatabaseErr err(DatabaseErr::DB_SUCCESS);
    if (name.empty()) {
        err.setErrorCode(DatabaseErr::DBERR_INVALID_INPUT);
        std::cerr << "DB: ERR: empty name provided\n";
    }
    return err;
}

template <typename T>
const DatabaseErr Database<T>::checkIfExists(const std::string& name) const
{
    DatabaseErr err(DatabaseErr::DB_SUCCESS);
    if (internalMaps.find(name) != internalMaps.end()) {
        err.setErrorCode(DatabaseErr::DBERR_ENTRY_ALREADY_EXISTS);
        std::cerr << "DB: ERR: entry already exists: " << name << "\n";
    }
    return err;
}

template <typename T>
const DatabaseErr Database<T>::addUniqueElement(const std::string& name, const T& obj)
{
    DatabaseErr err = checkIfValid(name);
    if (err.getErrorCode()) {
        return err;
    }

    err = checkIfExists(name);
    if (err.getErrorCode()) {
        return err;
    }

    internalMaps.emplace(name, obj);
    return DatabaseErr(DatabaseErr::DB_SUCCESS);
}

template <typename T>
const T* Database<T>::getElement(const std::string& name) const
{
    auto it = internalMaps.find(name);
    if (it == internalMaps.end()) {
        return nullptr;
    }
    return &it->second;
}

template <typename T>
size_t Database<T>::size() const
{
    return internalMaps.size();
}
