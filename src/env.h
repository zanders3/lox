#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include "value.h"

class Token;

class Environment
{
public:
    Environment();
    Value Get(const Token* name) const;
    void Assign(const Token* name, const Value& value);
    void Define(const Token* name, const Value& value);
    void DefineFunction(const std::string& name, LoxFunction function, int arity);
    void Push();
    void Pop();

private:
    std::vector<std::unordered_map<std::string,Value>> m_envs;
};
