#pragma once
#include <string>
#include <unordered_map>
#include "value.h"

class Token;

class Environment
{
public:
    Environment();
    Value Get(const Token* name) const;
    void Assign(const Token* name, const Value& value);
    void Define(const Token* name, const Value& value);

private:
    std::unordered_map<std::string,Value> m_values;
};
