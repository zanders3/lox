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

    Value VisitCall(const ExprCall& expr) 
    {
        Value callee = expr.callee->Visit(*this);
        if (callee.type != ValueType::FUNCTION || !callee.functionValue)
        {
            lox_error(*expr.paren, "Callee is not a function");
            return Value();
        }

        if (callee.intValue != expr.args.size())
        {
            char buf[64];
            std::snprintf(buf, 64, "Expected %d args but got %d", callee.intValue, (int)expr.args.size());
            lox_error(*expr.paren, buf);
        }

        std::vector<Value> args;
        for (const std::unique_ptr<Expr>& arg : expr.args)
            args.push_back(arg->Visit(*this));

        return callee.functionValue(*this, args, callee.functionStmt);
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
        if (stmt.initialiser)
            value = stmt.initialiser->Visit(*this);
        environment.Define(stmt.name, value);
    }

    void VisitBlock(const StmtBlock& stmt) 
    {
        environment.Push();

        for (const std::unique_ptr<Stmt>& stmt : stmt.stmts)
            if (stmt)
                stmt->Visit(*this);

        environment.Pop();
    }

    static Value FunctionCall(Interpreter& interpreter, std::vector<Value>& args, const StmtFunction* stmtFunction)
    {
        interpreter.environment.Push();
        for (int i = 0; i<args.size(); ++i)
                interpreter.environment.Define(stmtFunction->params[i], args[i]);
            
            for (const std::unique_ptr<Stmt>& stmt : stmtFunction->body)
                if (stmt)
                    stmt->Visit(interpreter);
        interpreter.environment.Pop();

        return Value();
    }

    void VisitFunction(const StmtFunction& stmt) 
    {
        environment.Define(stmt.name, Value(stmt.name->stringLiteral, Interpreter::FunctionCall, stmt.params.size(), &stmt));
    }

    void VisitIf(const StmtIf& stmt) 
    {
    	if (IsTruthy(stmt.condition->Visit(*this)))
    		stmt.thenBranch->Visit(*this);
    	else if (stmt.elseBranch)
    		stmt.elseBranch->Visit(*this);
    }

    void VisitPrint(const StmtPrint& expr) 
    {
        expr.expr->Visit(*this).Print();
    }

    void VisitReturn(const StmtReturn&) 
    {

    }

    void VisitWhile(const StmtWhile& stmt) 
    {
    	while (IsTruthy(stmt.condition->Visit(*this)))
    		stmt.body->Visit(*this);
    }

private:
    Environment& environment;
};

void lox_run(Environment& env, const char* source, int sourceLen)
{
    std::vector<Token> tokens;
    scanner_scan(source, sourceLen, tokens);

    std::vector<std::unique_ptr<Stmt>> stmts;
    parser_parse(tokens, stmts);

    Interpreter interpreter(env);
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
