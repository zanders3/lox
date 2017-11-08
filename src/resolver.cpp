#include "resolver.h"
#include "lox.h"
#include <unordered_map>
#include <vector>
#include <string>

typedef std::unordered_map<std::string,bool> ScopeMap;

struct Resolver : public ExprVisitor<void>, StmtVisitor<void>
{
	std::vector<ScopeMap> scopes;

	ScopeMap& PeekScope() {	return scopes[scopes.size() - 1]; }
	bool HasScope() { return scopes.size() > 0; }

	void Declare(const std::string& name)
	{
		if (HasScope())
			PeekScope().emplace(name, false);
	}

	void Define(const std::string& name)
	{
		if (HasScope())
		{
			PeekScope().erase(name);
			PeekScope().emplace(name, true);
		}
	}

	void ResolveLocal(const Expr& expr, const Token* name)
	{
		for (int i = scopes.size() - 1; i >= 0; --i)
		{
			if (scopes[i].find(name->lexeme) != scopes[i].end())
			{
				printf("resolve %s -> %i\n", name->lexeme, (int)scopes.size() - 1 - i);
				return;
			}
		}
	}

    void VisitBinary(const ExprBinary& expr) override
    {
    	VisitExpr(*expr.left);
    	VisitExpr(*expr.right);
    }

    void VisitCall(const ExprCall& expr) override
    {
    	VisitExpr(*expr.callee);
    	for (const ExprPtr& arg : expr.args)
    		VisitExpr(*arg);
    }

    void VisitGrouping(const ExprGrouping& group) override
    {
    	VisitExpr(*group.expr);
    }

    void VisitLiteral(const ExprLiteral& lit) override
    {
    }

    void VisitLogical(const ExprLogical& expr) override
    {
    	VisitExpr(*expr.left);
    	VisitExpr(*expr.right);
    }

    void VisitUnary(const ExprUnary& expr) override
    {
    	VisitExpr(*expr.right);
    }

    void VisitVariable(const ExprVariable& expr) override
    {
    	if (HasScope())
    	{
    		ScopeMap& scope = PeekScope();
    		auto item = scope.find(expr.name->lexeme);
    		if (item != scope.end() && item->second == false)
    		{
    			lox_error(*expr.name, "Cannot read local variable its own initialiser");
    		}
    	}

    	ResolveLocal(expr, expr.name);
    }

    void VisitAssign(const ExprAssign& expr) override
    {
    	VisitExpr(*expr.value);
    	ResolveLocal(expr, expr.name);
    }

    void VisitExpression(const StmtExpression& expr) override
    {
    	VisitExpr(*expr.expr);
    }

    void VisitVar(const StmtVar& stmt) override
    {
    	Declare(stmt.name->stringLiteral);
    	if (stmt.init)
    		VisitExpr(*stmt.init);
    	Define(stmt.name->stringLiteral);
    }

    void ExecuteBlock(const StmtPtrList& stmts)
    {
    	for (const StmtPtr& stmt : stmts)
    		if (stmt)
	    		VisitStmt(*stmt);
    }

    void VisitBlock(const StmtBlock& stmt) override
    {
    	scopes.emplace_back();
    	ExecuteBlock(stmt.stmts);
    	scopes.pop_back();
    }

    void VisitFunction(const StmtFunction& stmt) override
    {
    	Define(stmt.name->lexeme);

    	scopes.emplace_back();
    	for (const Token* param : stmt.params)
    		Define(param->lexeme);
    	ExecuteBlock(stmt.body);
    	scopes.pop_back();
    }

    void VisitIf(const StmtIf& stmt) override
    {
    	VisitExpr(*stmt.condition);
    	VisitStmt(*stmt.thenBranch);
    	if (stmt.elseBranch)
    		VisitStmt(*stmt.elseBranch);
    }

    void VisitPrint(const StmtPrint& expr) override
    {
    	VisitExpr(*expr.expr);
    }

    void VisitReturn(const StmtReturn& stmt) override
    {
    	if (stmt.value)
	    	VisitExpr(*stmt.value);
    }

    void VisitWhile(const StmtWhile& stmt) override
    {
    	VisitExpr(*stmt.condition);
    	VisitStmt(*stmt.body);
    }
};

void resolver_resolve(const StmtPtrList& stmts)
{
	Resolver resolver;
	resolver.ExecuteBlock(stmts);
}
