#pragma once
#include <memory>
#include <vector>
#include <string>

struct Value;
struct Interpreter;
struct Expr;
struct ExprCall;
struct StmtFunction;
class Environment;

typedef Value (*LoxFunction)(Interpreter& interpreter, std::vector<Value>& args);

struct Function
{
	Function(const std::string& name, LoxFunction function, const StmtFunction* stmt, int arity, const std::shared_ptr<Environment>& closure);

    std::string name;
    LoxFunction function;
    const StmtFunction* stmt;
    std::shared_ptr<Environment> closure;
    int arity;

    Value Call(Interpreter& interpreter, const ExprCall& expr);
};
