#pragma once
#include <string>
#include "function.h"

enum class ValueType
{
    NIL, BOOL, NUMBER, STRING, FUNCTION, ERROR
};

struct Value;
struct Interpreter;
struct ExprLiteral;

struct Value
{
    static const Value Error;

    Value();
    Value(bool value);
    Value(int value);
    Value(const std::string& value);
    Value(const std::shared_ptr<Function>& function);
    Value(const ExprLiteral& literal);
private:
    explicit Value(ValueType type);
public:

    ValueType type;
    std::string stringValue;
    int intValue;
    std::shared_ptr<Function> functionValue;

    void Print() const;
    int ToInt() const;
    bool IsValid() const { return type != ValueType::ERROR; }
    bool IsError() const { return type == ValueType::ERROR; }
};
