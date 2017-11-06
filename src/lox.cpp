#include "lox.h"
#include "scanner.h"
#include "parser.h"
#include "env.h"

struct Interpreter : public Visitor
{
    Interpreter(Environment& env)
        : environment(env)
    {}

    bool IsEqual(const Value& left, const Value& right) const
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

    Value VisitBinary(const ExprBinary& expr)
    {
        Value left = expr.left->Visit(*this);
        Value right = expr.right->Visit(*this);
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

    Value VisitCall(const ExprCall&) 
    {
        return Value();
    }

    Value VisitGrouping(const ExprGrouping& group)
    {
        return group.expr->Visit(*this);
    }

    Value VisitLiteral(const ExprLiteral& lit) 
    {
        return lit.value;
    }

    static bool IsTruthy(const Value& val)
    {
        if (val.type == ValueType::NIL) return false;
        if (val.type == ValueType::STRING) return true;
        return val.intValue > 0;
    }

    Value VisitLogical(const ExprLogical& expr) 
    {
        Value left = expr.left->Visit(*this);
        if (expr.op->type == TokenType::OR)
        {
            if (IsTruthy(left)) return left;
        }
        else
        {
            if (!IsTruthy(left)) return left;
        }
        
        return expr.right->Visit(*this);
    }
    
    Value VisitUnary(const ExprUnary& expr)
    {
        Value right = expr.right->Visit(*this);
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

    void VisitExpression(const StmtExpression& expr) 
    {
        expr.expr->Visit(*this);
    }

    Value VisitVariable(const ExprVariable& expr) 
    {
        return environment.Get(expr.name);
    }

    Value VisitAssign(const ExprAssign& expr)
    {
        Value value = expr.value->Visit(*this);
        environment.Assign(expr.name, value);
        return value;
    }

    void VisitVar(const StmtVar& stmt)
    {
        Value value;
        if (stmt.initializer)
            value = stmt.initializer->Visit(*this);
        environment.Define(stmt.name, value);
    }

    void VisitBlock(const StmtBlock&) 
    {

    }

    void VisitFunction(const StmtFunction&) 
    {

    }

    void VisitIf(const StmtIf&) 
    {

    }

    void VisitPrint(const StmtPrint& expr) 
    {
        expr.expr->Visit(*this).Print();
    }

    void VisitReturn(const StmtReturn&) 
    {

    }

    void VisitWhile(const StmtWhile&) 
    {

    }
private:
    Environment& environment;
};

Environment g_environment;

void lox_run(const char* source, int sourceLen)
{
    std::vector<Token> tokens;
    scanner_scan(source, sourceLen, tokens);

    std::vector<std::unique_ptr<Stmt>> stmts;
    parser_parse(tokens, stmts);

    Interpreter interpreter(g_environment);
    for (const std::unique_ptr<Stmt>& stmt : stmts)
    {
        if (stmt)
            stmt->Visit(interpreter);
    }
    printf("\n");
}

void lox_error(int line, const char* message)
{
    printf("[line %d] Error %s\n", line, message);
}

void lox_error(const Token& token, const char* message)
{
    if (token.type == TokenType::END)
        printf("[line %d] Error %s at end\n", token.line, message);
    else
        printf("[line %d] Error %s at %s\n", token.line, message, token.lexeme);
}
