#include "value.h"

Value::Value()
    : type(ValueType::NIL)
    , intValue(0)
{}
Value::Value(bool value)
    : type(ValueType::BOOL)
    , intValue(value)
{}
Value::Value(int value)
    : type(ValueType::NUMBER)
    , intValue(value)
{}
Value::Value(const std::string& value)
    : type(ValueType::STRING)
    , stringValue(value)
    , intValue(0)
{}

void Value::Print() const
{
    switch (type)
    {
        case ValueType::BOOL:
            printf(intValue ? "true\n" : "false\n");
            break;
        case ValueType::NUMBER:
            printf("%d\n", intValue);
            break;
        case ValueType::STRING:
            printf("%s\n", stringValue.c_str());
            break;
        case ValueType::NIL:
            printf("nil\n");
            break;
    }
}
