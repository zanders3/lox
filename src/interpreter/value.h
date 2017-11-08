#pragma once
#include <string>
#include "function.h"

enum class ValueType
{
    NIL, BOOL, NUMBER, STRING, FUNCTION, CLASS, INSTANCE, ERROR
};

struct Value;
struct Interpreter;
struct ExprLiteral;
struct LoxClass;

struct Value
{
    static const Value Error;

    Value();
    Value(bool value);
    Value(int value);
    Value(const std::string& value);
    Value(std::shared_ptr<LoxObject>&& function, ValueType type);
    Value(const ExprLiteral& literal);
private:
    explicit Value(ValueType type);
public:

    ValueType type;
    std::string stringValue;
    int intValue;
    std::shared_ptr<LoxObject> objectValue;

    Function* GetFunction();
    LoxClass* GetClass();
    LoxInstance* GetInstance();

    void Print() const;
    int ToInt() const;
    bool IsValid() const { return type != ValueType::ERROR; }
    bool IsError() const { return type == ValueType::ERROR; }
};
