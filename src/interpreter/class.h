#pragma once
#include <string>
#include <memory>

struct Interpreter;
struct Value;
struct ExprCall;

struct LoxObject
{
	virtual ~LoxObject();
};

struct LoxClass : public LoxObject
{
	LoxClass(const std::string& name);

	std::string name;
};

struct LoxInstance : public LoxObject
{
	LoxInstance(std::shared_ptr<LoxClass>&& loxClass);

	std::shared_ptr<LoxClass> loxClass;
};
