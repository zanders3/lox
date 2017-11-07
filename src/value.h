#pragma once
#include <string>

enum class ValueType
{
    NIL, BOOL, NUMBER, STRING, FUNCTION
};

struct Value;
struct Interpreter;
class StmtFunction;

typedef Value (*LoxFunction)(Interpreter& interpreter, std::vector<Value>& args, const StmtFunction* stmt);

struct Value
{
    Value();
    Value(bool value);
    Value(int value);
    Value(const std::string& value);
    Value(const std::string& name, LoxFunction function, int arity, const StmtFunction* functionStmt = nullptr);

    ValueType type;
    std::string stringValue;
    int intValue;
    LoxFunction functionValue;
    const StmtFunction* functionStmt;

    void Print() const;
    int ToInt() const;
};
