#include "value.h"
#include "ast.h"

const Value Value::Error = Value(ValueType::ERROR);

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
Value::Value(const std::shared_ptr<Function>& function)
    : type(ValueType::FUNCTION)
    , intValue(0)
    , functionValue(function)
{}
Value::Value(const ExprLiteral& literal)
    : stringValue(literal.stringValue)
    , intValue(literal.intValue)
{
    switch (literal.litType)
    {
        case LitType::Int: type = ValueType::NUMBER; break;
        case LitType::Bool: type = ValueType::BOOL; break;
        case LitType::String: type = ValueType::STRING; break;
        default:
        case LitType::Nil: type = ValueType::NIL; break;
    }
}
Value::Value(ValueType type)
    : type(type)
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
        case ValueType::FUNCTION:
            printf("func %s\n", functionValue ? functionValue->name.c_str() : "<nil>");
            break;
        case ValueType::ERROR:
            printf("<error>\n");
            break;
    }
}
