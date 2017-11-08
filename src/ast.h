#pragma once
#include "scanner.h"
#include <array>
#include <memory>

enum class ExprType
{
    Assign, Binary, Call, Grouping, Literal, Logical, Unary, Variable
};

struct Expr
{
    ExprType type;

    virtual ~Expr();
};

typedef std::unique_ptr<Expr> ExprPtr;
typedef std::vector<std::unique_ptr<Expr>> ExprPtrList;

struct ExprAssign : public Expr
{
    ExprAssign(const Token* name, ExprPtr&& value)
        : name(name)
        , value(std::move(value))
    {
        type = ExprType::Assign;
    }
    const Token* name;
    ExprPtr value;
};

struct ExprBinary : public Expr
{
    ExprBinary(ExprPtr&& left, const Token* op, ExprPtr&& right)
        : left(std::move(left))
        , op(op)
        , right(std::move(right))
    {
        type = ExprType::Binary;
    }

    ExprPtr left;
    const Token* op;
    ExprPtr right;
};

struct ExprCall : public Expr
{
    ExprCall(ExprPtr&& callee, const Token* paren, ExprPtrList&& args)
        : callee(std::move(callee))
        , paren(paren)
        , args(std::move(args))
    {
        type = ExprType::Call;
    }

    ExprPtr callee;
    const Token* paren;
    ExprPtrList args;
};

struct ExprGrouping : public Expr
{
    ExprGrouping(ExprPtr&& expr)
        : expr(std::move(expr))
    {
        type = ExprType::Grouping;
    }

    ExprPtr expr;
};

enum class LitType
{
    Int, Bool, String, Nil
};

struct ExprLiteral : public Expr
{
    explicit ExprLiteral(bool value)
        : litType(LitType::Bool)
        , intValue(value)
    {
        type = ExprType::Literal;
    }
    explicit ExprLiteral(int value)
        : litType(LitType::Int)
        , intValue(value)
    {
        type = ExprType::Literal;   
    }
    explicit ExprLiteral(const std::string& value)
        : litType(LitType::String)
        , intValue(0)
        , stringValue(value)
    {
        type = ExprType::Literal;
    }
    ExprLiteral()
        : litType(LitType::Nil)
        , intValue(0)
    {
        type = ExprType::Literal;
    }

    LitType litType;
    int intValue;
    std::string stringValue;
};

struct ExprLogical : public Expr
{
    ExprLogical(ExprPtr&& left, const Token* op, ExprPtr&& right)
        : left(std::move(left))
        , right(std::move(right))
        , op(op)
    {
        type = ExprType::Logical;   
    }

    ExprPtr left, right;
    const Token* op;
};

struct ExprUnary : public Expr
{
    ExprUnary(const Token* op, ExprPtr&& right)
        : right(std::move(right))
        , op(op)
    {
        type = ExprType::Unary;   
    }

    ExprPtr right;
    const Token* op;
};

struct ExprVariable : public Expr
{
    ExprVariable(const Token* name)
        : name(name)
    {
        type = ExprType::Variable;   
    }

    const Token* name;
};

template <typename Ret> struct ExprVisitor
{
    virtual ~ExprVisitor() {}

    virtual Ret VisitAssign(const ExprAssign& expr) = 0;
    virtual Ret VisitBinary(const ExprBinary& expr) = 0;
    virtual Ret VisitCall(const ExprCall& expr) = 0;
    virtual Ret VisitGrouping(const ExprGrouping& expr) = 0;
    virtual Ret VisitLiteral(const ExprLiteral& expr) = 0;
    virtual Ret VisitLogical(const ExprLogical& expr) = 0;
    virtual Ret VisitUnary(const ExprUnary& expr) = 0;
    virtual Ret VisitVariable(const ExprVariable& expr) = 0;

    Ret VisitExpr(const Expr& expr)
    {
        switch (expr.type)
        {
            case ExprType::Assign: return VisitAssign(static_cast<const ExprAssign&>(expr));
            case ExprType::Binary: return VisitBinary(static_cast<const ExprBinary&>(expr));
            case ExprType::Call: return VisitCall(static_cast<const ExprCall&>(expr));
            case ExprType::Grouping: return VisitGrouping(static_cast<const ExprGrouping&>(expr));
            case ExprType::Literal: return VisitLiteral(static_cast<const ExprLiteral&>(expr));
            case ExprType::Logical: return VisitLogical(static_cast<const ExprLogical&>(expr));
            case ExprType::Unary: return VisitUnary(static_cast<const ExprUnary&>(expr));
            case ExprType::Variable: return VisitVariable(static_cast<const ExprVariable&>(expr));
            default: return Ret();
        }
    }
};

enum class StmtType
{
    Block, Expression, Function, If, Print, Return, Var, While
};

struct Stmt
{
    StmtType type;

    virtual ~Stmt();
};

typedef std::unique_ptr<Stmt> StmtPtr;
typedef std::vector<std::unique_ptr<Stmt>> StmtPtrList;

struct StmtBlock : public Stmt
{
    StmtBlock(StmtPtrList&& stmts)
        : stmts(std::move(stmts))
    {
        type = StmtType::Block;   
    }

    StmtPtrList stmts;
};

struct StmtExpression : public Stmt
{
    StmtExpression(ExprPtr&& expr)
        : expr(std::move(expr))
    {
        type = StmtType::Expression;
    }

    ExprPtr expr;
};

struct StmtFunction : public Stmt
{
    StmtFunction(const Token* name, const std::vector<const Token*>&& params, StmtPtrList&& body)
        : name(name)
        , params(params)
        , body(std::move(body))
    {
        type = StmtType::Function;   
    }

    const Token* name;
    std::vector<const Token*> params;
    StmtPtrList body;
};

struct StmtIf : public Stmt
{
    StmtIf(ExprPtr&& condition, StmtPtr&& thenBranch, StmtPtr&& elseBranch)
        : condition(std::move(condition))
        , thenBranch(std::move(thenBranch))
        , elseBranch(std::move(elseBranch))
    {
        type = StmtType::If;
    }

    ExprPtr condition;
    StmtPtr thenBranch, elseBranch;
};

struct StmtPrint : public Stmt
{
    StmtPrint(ExprPtr&& expr)
        : expr(std::move(expr))
    {
        type = StmtType::Print;
    }

    ExprPtr expr;
};

struct StmtReturn : public Stmt
{
    StmtReturn(const Token* keyword, ExprPtr&& value)
        : keyword(keyword)
        , value(std::move(value))
    {
        type = StmtType::Return;
    }

    const Token* keyword;
    ExprPtr value;
};

struct StmtVar : public Stmt
{
    StmtVar(const Token* name, ExprPtr&& init)
        : name(name)
        , init(std::move(init))
    {
        type = StmtType::Var;
    }

    const Token* name;
    ExprPtr init;
};

struct StmtWhile : public Stmt
{
    StmtWhile(ExprPtr&& condition, StmtPtr&& body)
        : condition(std::move(condition))
        , body(std::move(body))
    {
        type = StmtType::While;
    }

    ExprPtr condition;
    StmtPtr body;
};

template <typename Ret> struct StmtVisitor
{
    virtual ~StmtVisitor() {}

    virtual Ret VisitBlock(const StmtBlock& stmt) = 0;
    virtual Ret VisitExpression(const StmtExpression& stmt) = 0;
    virtual Ret VisitFunction(const StmtFunction& stmt) = 0;
    virtual Ret VisitIf(const StmtIf& stmt) = 0;
    virtual Ret VisitPrint(const StmtPrint& stmt) = 0;
    virtual Ret VisitReturn(const StmtReturn& stmt) = 0;
    virtual Ret VisitVar(const StmtVar& stmt) = 0;
    virtual Ret VisitWhile(const StmtWhile& stmt) = 0;

    Ret VisitStmt(const Stmt& stmt)
    {
        switch (stmt.type)
        {
            case StmtType::Block: return VisitBlock(static_cast<const StmtBlock&>(stmt));
            case StmtType::Expression: return VisitExpression(static_cast<const StmtExpression&>(stmt));
            case StmtType::Function: return VisitFunction(static_cast<const StmtFunction&>(stmt));
            case StmtType::If: return VisitIf(static_cast<const StmtIf&>(stmt));
            case StmtType::Print: return VisitPrint(static_cast<const StmtPrint&>(stmt));
            case StmtType::Return: return VisitReturn(static_cast<const StmtReturn&>(stmt));
            case StmtType::Var: return VisitVar(static_cast<const StmtVar&>(stmt));
            case StmtType::While: return VisitWhile(static_cast<const StmtWhile&>(stmt));
            default: return Ret();
        }
    }
};
