#pragma once

#include "ast.h"
#include "value.h"
#include <memory>

class Environment;

struct Interpreter : public StmtVisitor<void>, ExprVisitor<Value>
{
    Interpreter(const std::shared_ptr<Environment>& env);

    Value VisitBinary(const ExprBinary& expr) override;
    Value VisitCall(const ExprCall& expr) override;
    Value VisitGrouping(const ExprGrouping& group) override;
    Value VisitLiteral(const ExprLiteral& lit) override;
    Value VisitLogical(const ExprLogical& expr) override;
    Value VisitUnary(const ExprUnary& expr) override;
    void VisitExpression(const StmtExpression& expr) override;
    Value VisitVariable(const ExprVariable& expr) override;
    Value VisitAssign(const ExprAssign& expr) override;

    void VisitVar(const StmtVar& stmt) override;
    void ExecuteBlock(const StmtPtrList& stmts);
    void VisitBlock(const StmtBlock& stmt) override;
    void VisitFunction(const StmtFunction& stmt) override;
    void VisitIf(const StmtIf& stmt) override;
    void VisitPrint(const StmtPrint& expr) override;
    void VisitReturn(const StmtReturn& stmt) override;
    void VisitWhile(const StmtWhile& stmt) override;

    Value returnValue;
    std::shared_ptr<Environment> environment;
    bool hadReturn = false;
};
