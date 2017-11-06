//AST(rettype, parent, name, vars)

AST(Value, Expr, Assign, const Token* name; std::unique_ptr<Expr> value;,return )
AST(Value, Expr, Binary, std::unique_ptr<Expr> left; const Token* op; std::unique_ptr<Expr> right;,return )
AST(Value, Expr, Call, std::unique_ptr<Expr> callee; const Token* paren; std::vector<std::unique_ptr<Expr>> args;,return )
AST(Value, Expr, Grouping, std::unique_ptr<Expr> expr;,return )
AST(Value, Expr, Literal, Value value;,return )
AST(Value, Expr, Logical, std::unique_ptr<Expr> left; const Token* op; std::unique_ptr<Expr> right;,return )
AST(Value, Expr, Unary, const Token* op; std::unique_ptr<Expr> right;,return )
AST(Value, Expr, Variable, const Token* name;,return )

AST(void, Stmt, Block, std::vector<std::unique_ptr<Stmt>> stmts;,)
AST(void, Stmt, Expression, std::unique_ptr<Expr> expr;,)
AST(void, Stmt, Function, const Token* name; std::vector<const Token*> params; std::vector<std::unique_ptr<Stmt>> body;,)
AST(void, Stmt, If, std::unique_ptr<Expr> condition; std::unique_ptr<Stmt> thenBranch; std::unique_ptr<Stmt> elseBranch;,)
AST(void, Stmt, Print, std::unique_ptr<Expr> expr;,)
AST(void, Stmt, Return, const Token* keyword; std::unique_ptr<Expr> value;,)
AST(void, Stmt, Var, const Token* name; std::unique_ptr<Expr> initializer;,)
AST(void, Stmt, While, std::unique_ptr<Expr> condition; std::unique_ptr<Stmt> body;,)