#include "env.h"
#include "value.h"
#include "ast.h"
#include "lox.h"
#include <cassert>

typedef std::unordered_map<std::string,Value>::iterator MapIterator;
typedef std::unordered_map<std::string,Value>::const_iterator ConstMapIterator;

Environment::Environment()
{
	Push();
}

Value Environment::Get(const Token* token) const 
{
	for (int i = m_envs.size() - 1; i >= 0; --i)
	{
		ConstMapIterator val = m_envs[i].find(token->stringLiteral);
		if (val != m_envs[i].end())
			return val->second;	
	}
	
	lox_error(*token, "Undefined variable");
    return Value();
}

void Environment::Assign(const Token* token, const Value& value) 
{
	for (int i = m_envs.size() - 1; i >= 0; --i)
	{
		MapIterator val = m_envs[i].find(token->stringLiteral);
		if (val != m_envs[i].end())
		{
			val->second = value;	
			return;
		}
	}
	
	lox_error(*token, "Undefined variable");
}

void Environment::Define(const Token* token, const Value& value)
{
	m_envs[m_envs.size() - 1].emplace(token->stringLiteral, value);
}

void Environment::Push()
{
	m_envs.push_back(std::unordered_map<std::string,Value>());
}

void Environment::Pop()
{
	assert(m_envs.size() > 1);
	m_envs.pop_back();
}
