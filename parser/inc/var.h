#pragma once

#include <string>

class Var {
public:
    Var(const std::string& name);
    Var();

protected:
        const std::string name;
};