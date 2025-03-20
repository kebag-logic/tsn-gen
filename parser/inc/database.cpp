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