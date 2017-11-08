#include "env.h"
#include "value.h"
#include "ast.h"
#include "lox.h"
#include <cassert>

Environment::Environment(const std::shared_ptr<Environment>& parent)
	: m_parent(parent)
{
}

Value Environment::Get(const Token* token) const 
{
	auto val = m_vars.find(token->stringLiteral);
	if (val != m_vars.end())
		return val->second;

	if (m_parent)
		return m_parent->Get(token);
	
	lox_error(*token, "Undefined variable");
    return Value::Error;
}

bool Environment::Assign(const Token* token, const Value& value) 
{
	auto val = m_vars.find(token->stringLiteral);
	if (val != m_vars.end())
	{
		val->second = value;	
		return true;
	}

	if (m_parent)
		return m_parent->Assign(token, value);

	lox_error(*token, "Undefined variable");
	return false;
}

bool Environment::Define(const Token* token, const Value& value)
{
	auto val = m_vars.find(token->stringLiteral);
	if (val == m_vars.end())
	{
		m_vars.emplace(token->stringLiteral, value);
		return true;
	}

	lox_error(*token, "Variable already defined");
	return false;
}

void Environment::DefineFunction(const std::string& name, LoxFunction function, int arity, const StmtFunction* stmt, const std::shared_ptr<Environment>& closure)
{
	m_vars.emplace(name, Value(std::make_shared<Function>(name, function, stmt, arity, closure)));
}

