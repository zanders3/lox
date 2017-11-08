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

const int GlobalVariable = -1;
typedef std::unique_ptr<Expr> ExprPtr;
typedef std::vector<std::unique_ptr<Expr>> ExprPtrList;

struct ExprAssign : public Expr
{
    ExprAssign(const Token* name, ExprPtr&& value)
        : name(name)
        , value(std::move(value))
        , depth(GlobalVariable)
    {
        type = ExprType::Assign;
    }
    const Token* name;
    ExprPtr value;
    int depth, idx;
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
        , depth(GlobalVariable)
    {
        type = ExprType::Variable;   
    }

    const Token* name;
    int depth, idx;
};

enum class StmtType
{
    Block, Expression, Function, If, Print, Return, Var, While, Class
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

typedef std::unique_ptr<StmtFunction> StmtFunctionPtr;
typedef std::vector<std::unique_ptr<StmtFunction>> StmtFunctionPtrList;

struct StmtClass : public Stmt
{
    StmtClass(const Token* name, StmtFunctionPtrList&& methods)
        : name(name)
        , methods(std::move(methods))
    {
        type = StmtType::Class;   
    }

    const Token* name;
    StmtFunctionPtrList methods;
};
