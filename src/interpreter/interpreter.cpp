#include "interpreter.h"
#include "lox.h"
#include "env.h"
#include "class.h"

Interpreter::Interpreter(const std::shared_ptr<Environment>& env)
    : environment(env)
    , globals(env)
{}

static bool IsEqual(const Value& left, const Value& right)
{
    if (left.type == ValueType::NIL && right.type == ValueType::NIL) return true;
    if (left.type == ValueType::NIL) return false;
    
    if (left.type == ValueType::STRING || right.type == ValueType::STRING)
    {
        if (left.type != ValueType::STRING || right.type != ValueType::STRING)
            return false;

        return left.stringValue == right.stringValue;
    }

    return left.intValue == right.intValue;
}

static bool CheckNumbers(const Token* op, const Value& left, const Value& right, bool error = true)
{
    if (left.type == ValueType::NUMBER && right.type == ValueType::NUMBER)
        return true;

    if (error)
        lox_error(*op, "Operands must be numbers");
    return false;
}

static bool CheckNumbers(const Token* op, const Value& operand)
{
    if (operand.type == ValueType::NUMBER) return true;
    lox_error(*op, "Operand must be a number");
    return false;
}

Value Interpreter::VisitBinary(const ExprBinary& expr)
{
    Value left = VisitExpr(*expr.left);
    Value right = VisitExpr(*expr.right);
    switch (expr.op->type)
    {
        case TokenType::MINUS:
            if (CheckNumbers(expr.op, left, right))
                return left.intValue - right.intValue;
            break;
        case TokenType::PLUS:
            if (CheckNumbers(expr.op, left, right, false))
                return left.intValue + right.intValue;
            else if (left.type == ValueType::STRING)
            {
                const char* rightStr = nullptr;
                switch (right.type)
                {
                    case ValueType::NIL: return left;
                    case ValueType::BOOL: rightStr = right.intValue ? "true" : "false"; break;
                    case ValueType::NUMBER: rightStr = std::to_string(right.intValue).c_str(); break;
                    case ValueType::STRING: rightStr = right.stringValue.c_str(); break;
                    case ValueType::FUNCTION:
                    case ValueType::CLASS:
                    case ValueType::INSTANCE:
                    case ValueType::ERROR: 
                        return Value::Error;
                }

                return Value(left.stringValue + rightStr);
            }
            break;
        case TokenType::STAR:
            if (CheckNumbers(expr.op, left, right))
                return left.intValue * right.intValue;
            break;
        case TokenType::GREATER:
            if (CheckNumbers(expr.op, left, right))
                return Value(left.intValue > right.intValue);
            break;
        case TokenType::GREATER_EQUAL:
            if (CheckNumbers(expr.op, left, right))
                return Value(left.intValue >= right.intValue);
            break;
        case TokenType::LESS:
            if (CheckNumbers(expr.op, left, right))
                return Value(left.intValue < right.intValue);
            break;
        case TokenType::LESS_EQUAL:
            if (CheckNumbers(expr.op, left, right))
                return Value(left.intValue <= right.intValue);
            break;
        case TokenType::BANG_EQUAL:
            return !IsEqual(left, right);
        case TokenType::EQUAL_EQUAL:
            return IsEqual(left, right);
        default:
            lox_error(*expr.op, "Unknown operand");
            break;
    }
    return Value::Error;
}

Value Interpreter::VisitCall(const ExprCall& expr) 
{
    Value callee = VisitExpr(*expr.callee);
    
    if (callee.type == ValueType::FUNCTION && callee.objectValue)
        return callee.GetFunction()->Call(*this, expr);
    else if (callee.type == ValueType::CLASS && callee.objectValue)
    {
        if (expr.args.size() != 0)
        {
            lox_error(*expr.paren, "Expected 0 args");
            return Value::Error;
        }

        return Value(std::make_shared<LoxInstance>(std::static_pointer_cast<LoxClass>(callee.objectValue)), ValueType::INSTANCE);
    }

    lox_error(*expr.paren, "Callee is not a function");
    return Value::Error;
}

Value Interpreter::VisitGrouping(const ExprGrouping& group)
{
    return VisitExpr(*group.expr);
}

Value Interpreter::VisitLiteral(const ExprLiteral& lit) 
{
    return Value(lit);
}

static bool IsTruthy(const Value& val)
{
    if (val.type == ValueType::NIL) return false;
    if (val.type == ValueType::STRING) return true;
    return val.intValue > 0;
}

Value Interpreter::VisitLogical(const ExprLogical& expr) 
{
    Value left = VisitExpr(*expr.left);
    if (left.IsError())
        return Value::Error;
    if (expr.op->type == TokenType::OR)
    {
        if (IsTruthy(left)) return left;
    }
    else
    {
        if (!IsTruthy(left)) return left;
    }
    
    return VisitExpr(*expr.right);
}

Value Interpreter::VisitUnary(const ExprUnary& expr)
{
    Value right = VisitExpr(*expr.right);
    switch (expr.op->type)
    {
        case TokenType::MINUS:
            if (CheckNumbers(expr.op, right))
                return -right.intValue;
        case TokenType::BANG:
            return !IsTruthy(right);
        default:
            break;
    }
    return Value();
}

Value Interpreter::VisitVariable(const ExprVariable& expr) 
{
    if (expr.depth == GlobalVariable)
        return globals->GetAt(expr.name, 0);
    else
        return environment->GetAt(expr.name, expr.depth);
}

Value Interpreter::VisitAssign(const ExprAssign& expr)
{
    Value value = VisitExpr(*expr.value);
    if (expr.depth == GlobalVariable)
        globals->AssignAt(expr.name, value, 0);
    else
        environment->AssignAt(expr.name, value, expr.depth);
    return value;
}

bool Interpreter::VisitExpression(const StmtExpression& expr) 
{
    return VisitExpr(*expr.expr).IsValid();
}

bool Interpreter::VisitVar(const StmtVar& stmt)
{
    Value value;
    if (stmt.init)
        value = VisitExpr(*stmt.init);
    if (value.IsError())
        return false;
    return environment->Define(stmt.name, value);
}

bool Interpreter::ExecuteBlock(const StmtPtrList& stmts)
{
    for (const StmtPtr& stmt : stmts)
    {
        if (!stmt || !VisitStmt(*stmt))
            return false;
        if (hadReturn)
            return true;
    }

    return true;
}

bool Interpreter::VisitBlock(const StmtBlock& stmt) 
{
    std::shared_ptr<Environment> parent = environment;
    environment = std::make_shared<Environment>(parent);
    bool result = ExecuteBlock(stmt.stmts);
    environment = parent;
    return result;
}

bool Interpreter::VisitFunction(const StmtFunction& stmt) 
{
    environment->DefineFunction(stmt.name->stringLiteral, nullptr, stmt.params.size(), &stmt, environment);
    return true;
}

bool Interpreter::VisitIf(const StmtIf& stmt) 
{
	if (IsTruthy(VisitExpr(*stmt.condition)))
		return VisitStmt(*stmt.thenBranch);
	else if (stmt.elseBranch)
		return VisitStmt(*stmt.elseBranch);
    return true;
}

bool Interpreter::VisitPrint(const StmtPrint& expr) 
{
    Value value = VisitExpr(*expr.expr);
    if (value.IsError())
        return false;
    value.Print();
    return true;
}

bool Interpreter::VisitReturn(const StmtReturn& stmt) 
{
    returnValue = VisitExpr(*stmt.value);
    if (returnValue.IsError())
        return false;
    hadReturn = true;
    return true;
}

bool Interpreter::VisitWhile(const StmtWhile& stmt) 
{
	while (IsTruthy(VisitExpr(*stmt.condition)))
    {
		if (!VisitStmt(*stmt.body))
            return false;
    }
    return true;
}

bool Interpreter::VisitClass(const StmtClass& stmt)
{
    if (!environment->Define(stmt.name, Value()))
        return false;
    return environment->AssignAt(stmt.name, Value(std::make_shared<LoxClass>(stmt.name->lexeme), ValueType::CLASS), GlobalVariable);

}
