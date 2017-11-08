#include "env.h"
#include "value.h"
#include "ast.h"
#include "lox.h"
#include <cassert>

typedef std::unordered_map<std::string,Value>::iterator MapIterator;
typedef std::unordered_map<std::string,Value>::const_iterator ConstMapIterator;

Environment::Environment(const std::shared_ptr<Environment>& parent)
	: m_parent(parent)
{
}

Value Environment::Get(const Token* token) const 
{
	ConstMapIterator val = m_vars.find(token->stringLiteral);
	if (val != m_vars.end())
		return val->second;

	if (m_parent)
		return m_parent->Get(token);
	
	lox_error(*token, "Undefined variable");
    return Value();
}

void Environment::Assign(const Token* token, const Value& value) 
{
	MapIterator val = m_vars.find(token->stringLiteral);
	if (val != m_vars.end())
	{
		val->second = value;	
		return;
	}

	if (m_parent)
		m_parent->Assign(token, value);
	else
		lox_error(*token, "Undefined variable");
}

void Environment::Define(const Token* token, const Value& value)
{
	m_vars.emplace(token->stringLiteral, value);
}

void Environment::DefineFunction(const std::string& name, LoxFunction function, int arity, const StmtFunction* stmt, const std::shared_ptr<Environment>& closure)
{
	m_vars.emplace(name, Value(std::make_shared<Function>(name, function, stmt, arity, closure)));
}

