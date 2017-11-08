#include "interpreter.h"
#include "lox.h"
#include "env.h"

Interpreter::Interpreter(const std::shared_ptr<Environment>& env)
    : environment(env)
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
                    case ValueType::FUNCTION: rightStr = (std::string("func ") + right.stringValue).c_str(); break;
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
    return Value();
}

Value Interpreter::VisitCall(const ExprCall& expr) 
{
    Value callee = VisitExpr(*expr.callee);
    if (callee.type != ValueType::FUNCTION || !callee.functionValue)
    {
        lox_error(*expr.paren, "Callee is not a function");
        return Value();
    }

    return callee.functionValue->Call(*this, expr);
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

void Interpreter::VisitExpression(const StmtExpression& expr) 
{
    VisitExpr(*expr.expr);
}

Value Interpreter::VisitVariable(const ExprVariable& expr) 
{
    return environment->Get(expr.name);
}

Value Interpreter::VisitAssign(const ExprAssign& expr)
{
    Value value = VisitExpr(*expr.value);
    environment->Assign(expr.name, value);
    return value;
}

void Interpreter::VisitVar(const StmtVar& stmt)
{
    Value value;
    if (stmt.init)
        value = VisitExpr(*stmt.init);
    environment->Define(stmt.name, value);
}

void Interpreter::ExecuteBlock(const StmtPtrList& stmts)
{
    for (const StmtPtr& stmt : stmts)
    {
        if (stmt)
            VisitStmt(*stmt);
        if (g_hadError || hadReturn)
            return;
    }
}

void Interpreter::VisitBlock(const StmtBlock& stmt) 
{
    std::shared_ptr<Environment> parent = environment;
    environment = std::make_shared<Environment>(parent);
    ExecuteBlock(stmt.stmts);
    environment = parent;
}

void Interpreter::VisitFunction(const StmtFunction& stmt) 
{
    environment->DefineFunction(stmt.name->stringLiteral, nullptr, stmt.params.size(), &stmt, environment);
}

void Interpreter::VisitIf(const StmtIf& stmt) 
{
	if (IsTruthy(VisitExpr(*stmt.condition)))
		VisitStmt(*stmt.thenBranch);
	else if (stmt.elseBranch)
		VisitStmt(*stmt.elseBranch);
}

void Interpreter::VisitPrint(const StmtPrint& expr) 
{
    VisitExpr(*expr.expr).Print();
}

void Interpreter::VisitReturn(const StmtReturn& stmt) 
{
    returnValue = VisitExpr(*stmt.value);
    hadReturn = true;
}

void Interpreter::VisitWhile(const StmtWhile& stmt) 
{
	while (IsTruthy(VisitExpr(*stmt.condition)))
		VisitStmt(*stmt.body);
}
