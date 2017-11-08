#include "resolver.h"
#include "lox.h"
#include <unordered_map>
#include <vector>
#include <string>
#include "ast_visitors.h"

struct VariableScope
{
	int variableIdx;
	bool isDefined;
};

typedef std::unordered_map<std::string,VariableScope> ScopeMap;

enum class FunctionType
{
	None, Function
};

struct Resolver : public ExprVisitor<void>, StmtVisitor<void>
{
	ScopeMap& PeekScope() {	return scopes[scopes.size() - 1]; }
	bool HasScope() { return scopes.size() > 0; }

	void Declare(const Token& name)
	{
		ScopeMap& scope = HasScope() ? PeekScope() : globalScope;
		auto item = scope.find(name.lexeme);
		if (item == scope.end())
			scope.emplace(name.lexeme, VariableScope{ (int)scope.size(), false });
		else
		{
			lox_error(name, "Variable with this name already declared in this scope");
			hadError = true;
		}
	}

	void Define(const Token& name)
	{
		ScopeMap& scope = HasScope() ? PeekScope() : globalScope;
		auto item = scope.find(name.lexeme);
		if (item == scope.end())
		{
			lox_error(name, "Variable not declared");
			hadError = true;
		}
		else
			item->second.isDefined = true;
	}

	void ResolveVariable(const Token* name, int& outDepth, int& outIdx)
	{
		for (int i = scopes.size() - 1; i >= 0; --i)
		{
			auto item = scopes[i].find(name->lexeme);
			if (item != scopes[i].end())
			{
				outDepth = (int)scopes.size() - 1 - i;
				outIdx = item->second.variableIdx;
				return;
			}
		}
	}

    void VisitBinary(ExprBinary& expr) override
    {
    	VisitExpr(*expr.left);
    	VisitExpr(*expr.right);
    }

    void VisitCall(ExprCall& expr) override
    {
    	VisitExpr(*expr.callee);
    	for (const ExprPtr& arg : expr.args)
    		VisitExpr(*arg);
    }

    void VisitGrouping(ExprGrouping& group) override
    {
    	VisitExpr(*group.expr);
    }

    void VisitLiteral(ExprLiteral& lit) override
    {
    }

    void VisitLogical(ExprLogical& expr) override
    {
    	VisitExpr(*expr.left);
    	VisitExpr(*expr.right);
    }

    void VisitUnary(ExprUnary& expr) override
    {
    	VisitExpr(*expr.right);
    }

    void VisitVariable(ExprVariable& expr) override
    {
    	if (HasScope())
    	{
    		ScopeMap& scope = PeekScope();
    		auto item = scope.find(expr.name->lexeme);
    		if (item != scope.end() && item->second.isDefined == false)
    		{
    			lox_error(*expr.name, "Cannot read local variable its own initialiser");
    			hadError = true;
    		}
    	}

    	ResolveVariable(expr.name, expr.depth, expr.idx);
    }

    void VisitAssign(ExprAssign& expr) override
    {
    	VisitExpr(*expr.value);
    	ResolveVariable(expr.name, expr.depth, expr.idx);
    }

    void VisitExpression(StmtExpression& expr) override
    {
    	VisitExpr(*expr.expr);
    }

    void VisitVar(StmtVar& stmt) override
    {
    	Declare(*stmt.name);
    	if (stmt.init)
    		VisitExpr(*stmt.init);
    	Define(*stmt.name);
    }

    void ExecuteBlock(StmtPtrList& stmts)
    {
    	for (StmtPtr& stmt : stmts)
    		if (stmt)
	    		VisitStmt(*stmt);
    }

    void VisitBlock(StmtBlock& stmt) override
    {
    	scopes.emplace_back();
    	ExecuteBlock(stmt.stmts);
    	scopes.pop_back();
    }

    void VisitFunction(StmtFunction& stmt) override
    {
    	Declare(*stmt.name);
    	Define(*stmt.name);

    	FunctionType enclosingFunctionType = currentFunction;
    	currentFunction = FunctionType::Function;
    	scopes.emplace_back();
    	for (const Token* param : stmt.params)
    	{
    		Declare(*param);
    		Define(*param);
    	}
    	ExecuteBlock(stmt.body);
    	scopes.pop_back();
    	currentFunction = enclosingFunctionType;
    }

    void VisitIf(StmtIf& stmt) override
    {
    	VisitExpr(*stmt.condition);
    	VisitStmt(*stmt.thenBranch);
    	if (stmt.elseBranch)
    		VisitStmt(*stmt.elseBranch);
    }

    void VisitPrint(StmtPrint& expr) override
    {
    	VisitExpr(*expr.expr);
    }

    void VisitReturn(StmtReturn& stmt) override
    {
    	if (currentFunction == FunctionType::None)
    	{
    		lox_error(*stmt.keyword, "Cannot return at top level");
    		hadError = true;
    	}

    	if (stmt.value)
	    	VisitExpr(*stmt.value);
    }

    void VisitWhile(StmtWhile& stmt) override
    {
    	VisitExpr(*stmt.condition);
    	VisitStmt(*stmt.body);
    }

    void VisitClass(StmtClass& stmt) override
    {
    	Declare(*stmt.name);
    	Define(*stmt.name);
    }

 	std::vector<ScopeMap> scopes;
	ScopeMap globalScope;
	FunctionType currentFunction = FunctionType::None;
    bool hadError = false;
};

bool resolver_resolve(StmtPtrList& stmts)
{
	Resolver resolver;
	resolver.ExecuteBlock(stmts);

	return !resolver.hadError;
}
