#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include "value.h"

class Token;
class StmtFunction;

class Environment
{
public:
    Environment(const std::shared_ptr<Environment>& parent = std::shared_ptr<Environment>());
    Value Get(const Token* name) const;
    void Assign(const Token* name, const Value& value);
    void Define(const Token* name, const Value& value);
    void DefineFunction(const std::string& name, LoxFunction function, int arity, const StmtFunction* stmt = nullptr, const std::shared_ptr<Environment>& closure = std::shared_ptr<Environment>());

private:
    std::unordered_map<std::string,Value> m_vars;
    std::shared_ptr<Environment> m_parent;
};
