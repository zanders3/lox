#include "env.h"
#include "value.h"
#include "ast.h"
#include "lox.h"

typedef std::unordered_map<std::string,Value>::iterator MapIterator;
typedef std::unordered_map<std::string,Value>::const_iterator ConstMapIterator;

Environment::Environment()
{}

Value Environment::Get(const Token* token) const 
{
	ConstMapIterator val = m_values.find(token->stringLiteral);
	if (val != m_values.end())
		return val->second;

	lox_error(*token, "Undefined variable");
    return Value();
}

void Environment::Assign(const Token* token, const Value& value) 
{
	MapIterator val = m_values.find(token->stringLiteral);
	if (val != m_values.end())
		val->second = value;
	else
		lox_error(*token, "Undefined variable");
}

void Environment::Define(const Token* token, const Value& value)
{
	m_values.emplace(token->stringLiteral, value);
}
