#pragma once

#include "ast_visitors.h"
#include "value.h"
#include <memory>

class Environment;

struct Interpreter : public ConstStmtVisitor<bool>, ConstExprVisitor<Value>
{
    Interpreter(const std::shared_ptr<Environment>& env);

    Value VisitBinary(const ExprBinary& expr) override;
    Value VisitCall(const ExprCall& expr) override;
    Value VisitGrouping(const ExprGrouping& group) override;
    Value VisitLiteral(const ExprLiteral& lit) override;
    Value VisitLogical(const ExprLogical& expr) override;
    Value VisitUnary(const ExprUnary& expr) override;
    Value VisitVariable(const ExprVariable& expr) override;
    Value VisitAssign(const ExprAssign& expr) override;

    bool VisitExpression(const StmtExpression& expr) override;
    bool VisitVar(const StmtVar& stmt) override;
    bool ExecuteBlock(const StmtPtrList& stmts);
    bool VisitBlock(const StmtBlock& stmt) override;
    bool VisitFunction(const StmtFunction& stmt) override;
    bool VisitIf(const StmtIf& stmt) override;
    bool VisitPrint(const StmtPrint& expr) override;
    bool VisitReturn(const StmtReturn& stmt) override;
    bool VisitWhile(const StmtWhile& stmt) override;
    bool VisitClass(const StmtClass& stmt) override;

    Value returnValue;
    std::shared_ptr<Environment> environment;
    std::shared_ptr<Environment> globals;
    bool hadReturn = false;
};
