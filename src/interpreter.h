#pragma once

#include "ast.h"
#include <memory>

class Environment;

struct Interpreter : public Visitor
{
    Interpreter(const std::shared_ptr<Environment>& env);

    Value VisitBinary(const ExprBinary& expr);
    Value VisitCall(const ExprCall& expr);
    Value VisitGrouping(const ExprGrouping& group);
    Value VisitLiteral(const ExprLiteral& lit);
    Value VisitLogical(const ExprLogical& expr);
    Value VisitUnary(const ExprUnary& expr);
    void VisitExpression(const StmtExpression& expr);
    Value VisitVariable(const ExprVariable& expr);
    Value VisitAssign(const ExprAssign& expr);

    void VisitVar(const StmtVar& stmt);
    void ExecuteBlock(const std::vector<std::unique_ptr<Stmt>>& stmts);
    void VisitBlock(const StmtBlock& stmt);
    void VisitFunction(const StmtFunction& stmt);
    void VisitIf(const StmtIf& stmt);
    void VisitPrint(const StmtPrint& expr);
    void VisitReturn(const StmtReturn& stmt);
    void VisitWhile(const StmtWhile& stmt);

    Value returnValue;
    std::shared_ptr<Environment> environment;
    bool hadReturn = false;
};
