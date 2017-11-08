#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include "interpreter/value.h"

struct Token;
struct StmtFunction;

class Environment
{
public:
    Environment(const std::shared_ptr<Environment>& parent = std::shared_ptr<Environment>());
    Value GetAt(const Token* name, int depth) const;
    bool AssignAt(const Token* name, const Value& value, int depth);
    bool Define(const Token* name, const Value& value);
    void DefineFunction(const std::string& name, LoxFunction function, int arity, const StmtFunction* stmt = nullptr, const std::shared_ptr<Environment>& closure = std::shared_ptr<Environment>());

private:
    std::unordered_map<std::string,Value> m_vars;
    std::shared_ptr<Environment> m_parent;
};
