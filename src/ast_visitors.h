#pragma once

#include "ast.h"

template <typename Ret> struct ExprVisitor
{
    virtual ~ExprVisitor() {}

    virtual Ret VisitAssign(ExprAssign& expr) = 0;
    virtual Ret VisitBinary(ExprBinary& expr) = 0;
    virtual Ret VisitCall(ExprCall& expr) = 0;
    virtual Ret VisitGrouping(ExprGrouping& expr) = 0;
    virtual Ret VisitLiteral(ExprLiteral& expr) = 0;
    virtual Ret VisitLogical(ExprLogical& expr) = 0;
    virtual Ret VisitUnary(ExprUnary& expr) = 0;
    virtual Ret VisitVariable(ExprVariable& expr) = 0;

    Ret VisitExpr(Expr& expr)
    {
        switch (expr.type)
        {
            case ExprType::Assign: return VisitAssign(static_cast<ExprAssign&>(expr));
            case ExprType::Binary: return VisitBinary(static_cast<ExprBinary&>(expr));
            case ExprType::Call: return VisitCall(static_cast<ExprCall&>(expr));
            case ExprType::Grouping: return VisitGrouping(static_cast<ExprGrouping&>(expr));
            case ExprType::Literal: return VisitLiteral(static_cast<ExprLiteral&>(expr));
            case ExprType::Logical: return VisitLogical(static_cast<ExprLogical&>(expr));
            case ExprType::Unary: return VisitUnary(static_cast<ExprUnary&>(expr));
            case ExprType::Variable: return VisitVariable(static_cast<ExprVariable&>(expr));
            default: return Ret();
        }
    }
};

template <typename Ret> struct ConstExprVisitor
{
    virtual ~ConstExprVisitor() {}

    virtual Ret VisitAssign(const ExprAssign& expr) = 0;
    virtual Ret VisitBinary(const ExprBinary& expr) = 0;
    virtual Ret VisitCall(const ExprCall& expr) = 0;
    virtual Ret VisitGrouping(const ExprGrouping& expr) = 0;
    virtual Ret VisitLiteral(const ExprLiteral& expr) = 0;
    virtual Ret VisitLogical(const ExprLogical& expr) = 0;
    virtual Ret VisitUnary(const ExprUnary& expr) = 0;
    virtual Ret VisitVariable(const ExprVariable& expr) = 0;

    Ret VisitExpr(Expr& expr)
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


template <typename Ret> struct StmtVisitor
{
    virtual ~StmtVisitor() {}

    virtual Ret VisitBlock(StmtBlock& stmt) = 0;
    virtual Ret VisitExpression(StmtExpression& stmt) = 0;
    virtual Ret VisitFunction(StmtFunction& stmt) = 0;
    virtual Ret VisitIf(StmtIf& stmt) = 0;
    virtual Ret VisitPrint(StmtPrint& stmt) = 0;
    virtual Ret VisitReturn(StmtReturn& stmt) = 0;
    virtual Ret VisitVar(StmtVar& stmt) = 0;
    virtual Ret VisitWhile(StmtWhile& stmt) = 0;
    virtual Ret VisitClass(StmtClass& stmt) = 0;

    Ret VisitStmt(Stmt& stmt)
    {
        switch (stmt.type)
        {
            case StmtType::Block: return VisitBlock(static_cast<StmtBlock&>(stmt));
            case StmtType::Expression: return VisitExpression(static_cast<StmtExpression&>(stmt));
            case StmtType::Function: return VisitFunction(static_cast<StmtFunction&>(stmt));
            case StmtType::If: return VisitIf(static_cast<StmtIf&>(stmt));
            case StmtType::Print: return VisitPrint(static_cast<StmtPrint&>(stmt));
            case StmtType::Return: return VisitReturn(static_cast<StmtReturn&>(stmt));
            case StmtType::Var: return VisitVar(static_cast<StmtVar&>(stmt));
            case StmtType::While: return VisitWhile(static_cast<StmtWhile&>(stmt));
            case StmtType::Class: return VisitClass(static_cast<StmtClass&>(stmt));
            default: return Ret();
        }
    }
};

template <typename Ret> struct ConstStmtVisitor
{
    virtual ~ConstStmtVisitor() {}

    virtual Ret VisitBlock(const StmtBlock& stmt) = 0;
    virtual Ret VisitExpression(const StmtExpression& stmt) = 0;
    virtual Ret VisitFunction(const StmtFunction& stmt) = 0;
    virtual Ret VisitIf(const StmtIf& stmt) = 0;
    virtual Ret VisitPrint(const StmtPrint& stmt) = 0;
    virtual Ret VisitReturn(const StmtReturn& stmt) = 0;
    virtual Ret VisitVar(const StmtVar& stmt) = 0;
    virtual Ret VisitWhile(const StmtWhile& stmt) = 0;
    virtual Ret VisitClass(const StmtClass& stmt) = 0;

    Ret VisitStmt(Stmt& stmt)
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
            case StmtType::Class: return VisitClass(static_cast<const StmtClass&>(stmt));
            default: return Ret();
        }
    }
};
