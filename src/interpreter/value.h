#pragma once
#include <string>
#include "function.h"

enum class ValueType
{
    NIL, BOOL, NUMBER, STRING, FUNCTION
};

struct Value;
struct Interpreter;
struct ExprLiteral;

struct Value
{
    Value();
    Value(bool value);
    Value(int value);
    Value(const std::string& value);
    Value(const std::shared_ptr<Function>& function);
    Value(const ExprLiteral& literal);

    ValueType type;
    std::string stringValue;
    int intValue;
    std::shared_ptr<Function> functionValue;

    void Print() const;
    int ToInt() const;
};
