#pragma once
#include <string>

enum class ValueType
{
    NIL, BOOL, NUMBER, STRING, FUNCTION
};

struct Value;
struct Interpreter;

typedef Value (*LoxFunction)(Interpreter& interpreter, std::vector<Value>& args);

struct Value
{
    Value();
    Value(bool value);
    Value(int value);
    Value(const std::string& value);
    Value(const std::string& name, LoxFunction function, int arity);

    ValueType type;
    std::string stringValue;
    int intValue;
    LoxFunction functionValue;

    void Print() const;
    int ToInt() const;
};
