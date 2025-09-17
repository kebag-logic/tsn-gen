/*
 * SPDX-FileCopyrightText: 2025 Kebag-Logic (https://kebag-logic.com)
 * SPDX-FileCopyrightText: 2025 Alexandre Malki <alexandre.malki@kebag-logic.com>
 * SPDX-License-Identifier: MIT
 */
 

 /*
 * SPDX-FileCopyrightText: 2025 Kebag-Logic (https://kebag-logic.com)
 * SPDX-FileCopyrightText: 2025 Alexandre Malki <alexandre.malki@kebag-logic.com>
 * SPDX-License-Identifier: MIT
 */
 

 #include <database.h>

template <typename T>
Database<T>::Database()
{
}

template <typename T>
const DatabaseErr Database<T>::checkIfValid(ryml::NodeRef& node)
{
    DatabaseErr err(DatabaseErr::DB_SUCCESS);
    if (node.invalid()) {
        err.setErrorCode(DatabaseErr::DBERR_INVALID_INPUT);
    }

    return err;
}

template <typename T>
const DatabaseErr Database<T>::checkIfExists(ryml::NodeRef& node)
{
    DatabaseErr err(DatabaseErr::DB_SUCCESS);
    if (node.invalid()) {
        err.setErrorCode(DatabaseErr::DBERR_INVALID_INPUT);
    }

    return err;
}
