#pragma once
#include <string>

enum class ValueType
{
    NIL, BOOL, NUMBER, STRING
};

struct Value
{
    Value();
    Value(bool value);
    Value(int value);
    Value(const std::string& value);

    ValueType type;
    std::string stringValue;
    int intValue;

    void Print() const;
    int ToInt() const;
};
