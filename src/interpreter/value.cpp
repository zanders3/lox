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
Value::Value(std::shared_ptr<LoxObject>&& object, ValueType type)
    : type(type)
    , intValue(0)
    , objectValue(object)
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

Function* Value::GetFunction() 
{ 
    return static_cast<Function*>(objectValue.get());
}

LoxClass* Value::GetClass()
{ 
    return static_cast<LoxClass*>(objectValue.get()); 
}

LoxInstance* Value::GetInstance() 
{ 
    return static_cast<LoxInstance*>(objectValue.get()); 
}

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
            printf("func %s\n", objectValue ? static_cast<const Function*>(objectValue.get())->name.c_str() : "<nil>");
            break;
        case ValueType::CLASS:
            printf("class %s\n", objectValue ? static_cast<const LoxClass*>(objectValue.get())->name.c_str() : "<nil>");
            break;
        case ValueType::INSTANCE:
            printf("instance %s\n", objectValue ? static_cast<const LoxInstance*>(objectValue.get())->loxClass->name.c_str() : "<nil>");
            break;
        case ValueType::ERROR:
            printf("<error>\n");
            break;
    }
}
