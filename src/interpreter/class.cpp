#include "class.h"
#include "value.h"
#include "ast.h"
#include "lox.h"

LoxObject::~LoxObject()
{}

LoxClass::LoxClass(const std::string& name)
	: name(name)
{}

LoxInstance::LoxInstance(std::shared_ptr<LoxClass>&& loxClass)
	: loxClass(loxClass)
{}
