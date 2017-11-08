#include "function.h"
#include "value.h"
#include "interpreter.h"
#include "lox.h"
#include "env.h"

Function::Function(const std::string& name, LoxFunction function, const StmtFunction* stmt, int arity, const std::shared_ptr<Environment>& closure)
	: name(name)
	, function(function)
	, stmt(stmt)
	, closure(closure)
    , arity(arity)
{}

Value Function::Call(Interpreter& interpreter, const ExprCall& expr)
{
    if (arity != expr.args.size())
    {
        char buf[64];
        std::snprintf(buf, 64, "Expected %d args but got %d", arity, (int)expr.args.size());
        lox_error(*expr.paren, buf);
        return Value::Error;
    }

    std::vector<Value> args;
    for (const ExprPtr& arg : expr.args)
        args.push_back(interpreter.VisitExpr(*arg));

    if (function)
    	return function(interpreter, args);

    interpreter.returnValue = Value();
    interpreter.hadReturn = false;
    std::shared_ptr<Environment> original = interpreter.environment;
    interpreter.environment = std::make_shared<Environment>(closure);
    for (int i = 0; i<args.size(); ++i)
        interpreter.environment->Define(stmt->params[i], args[i]);
        
    interpreter.ExecuteBlock(stmt->body);  
    interpreter.environment = original;
    interpreter.hadReturn = false;

    return interpreter.returnValue;
}
