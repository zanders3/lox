#include "parser.h"
#include "lox.h"
#include "scanner.h"
#include "ast.h"

typedef std::unique_ptr<Stmt> StmtPtr;
typedef std::unique_ptr<Expr> ExprPtr;

struct Parser
{
    const std::vector<Token>& m_tokens;
    int m_current;

    Parser(const std::vector<Token>& tokens)
        : m_tokens(tokens)
        , m_current(0)
    {}

    inline const Token& Peek() { return m_tokens[m_current]; }
    inline const Token& Previous() { return m_tokens[m_current - 1]; }
    inline bool IsAtEnd() { return Peek().type == TokenType::END; }

    const Token& Advance()
    {
        if (!IsAtEnd()) ++m_current;
        return Previous();
    }

    bool Check(TokenType type)
    {
        if (IsAtEnd()) return false;
        return Peek().type == type;
    }

    bool Match(TokenType type)
    {
        if (Check(type))
        {
            Advance();
            return true;
        }
        return false;
    }

    const Token* Consume(TokenType type, const char* message)
    {
        if (Check(type)) return &Advance();

        lox_error(Peek(), message);
        return nullptr;
    }

    void Synchronize()
    {
        Advance();

        while (!IsAtEnd())
        {
            if (Previous().type == TokenType::SEMICOLON) 
                return;
            switch (Peek().type)
            {
                case TokenType::CLASS: case TokenType::FUN: case TokenType::VAR:
                case TokenType::IF: case TokenType::WHILE: case TokenType::PRINT: case TokenType::RETURN:
                    return;
                default:
                    Advance();
                    break;
            }
        }
    }

    // program -> declaration* EOF
    void Parse(std::vector<StmtPtr>& stmts)
    {
        while (!IsAtEnd())
        {
            StmtPtr stmt(Declaration());
            if (stmt)
                stmts.push_back(std::move(stmt));
        }
    }

    //declaration -> funcDecl | varDecl | statement
    StmtPtr Declaration()
    {
        StmtPtr stmt;
        //if (Match(TokenType::FUN)) stmt = Function();
        if (Match(TokenType::VAR)) stmt = VarDecl();
        else stmt = Statement();

        if (!stmt)
            Synchronize();

        return stmt;
    }

    // statement -> exprStmt | forStmt | ifStmt | printStmt | returnStmt | whileStmt | block
    StmtPtr Statement()
    {
        if (Match(TokenType::PRINT)) return PrintStatement();
        if (Match(TokenType::IF)) return IfStatement();
        if (Match(TokenType::WHILE)) return WhileStatement();
        if (Match(TokenType::LEFT_BRACE)) return BlockStatement();
        if (Match(TokenType::FOR)) return ForStatement();

        return ExpressionStatement();
    }

    // forStmt -> "for" "(" (varDecl | exprStmt | ";") expression? ";" expression? ")" statement
    StmtPtr ForStatement()
    {
        if (!Consume(TokenType::LEFT_BRACE, "Expect '(' after 'for'"))
            return StmtPtr();
        
        StmtPtr initialiser;
        if (Match(TokenType::SEMICOLON))
        {}
        else if (Match(TokenType::VAR))
            initialiser = VarDecl();
        else
            initialiser = ExpressionStatement();

        ExprPtr condition;
        if (!Check(TokenType::SEMICOLON))
            condition = Expression();
        if (!Consume(TokenType::SEMICOLON, "Expect ';' after loop condition"))
            return StmtPtr();

        ExprPtr increment;
        if (!Check(TokenType::RIGHT_PAREN))
            increment = Expression();
        Consume(TokenType::RIGHT_PAREN, "Expect ')' after for clause");

        StmtPtr body = Statement();

        if (increment)
        {
            std::unique_ptr<StmtExpression> incrementExpr(new StmtExpression());
            incrementExpr->expr = std::move(increment);

            std::unique_ptr<StmtBlock> stmt(new StmtBlock());
            stmt->stmts.push_back(std::move(body));
            stmt->stmts.push_back(std::move(incrementExpr));
            body = std::move(stmt);
        }

        if (!condition)
            condition = Literal(Value(true));

        std::unique_ptr<StmtWhile> whileStmt(new StmtWhile());
        whileStmt->condition = std::move(condition);
        whileStmt->body = std::move(body);
        body = std::move(whileStmt);

        if (initialiser)
        {
            std::unique_ptr<StmtBlock> stmt(new StmtBlock());
            stmt->stmts.push_back(std::move(initialiser));
            stmt->stmts.push_back(std::move(body));
            body = std::move(stmt);
        }

        return body;
    }

    // block -> declaration*
    StmtPtr BlockStatement()
    {
        std::vector<StmtPtr> stmts;
        while (!Check(TokenType::RIGHT_BRACE) && !IsAtEnd())
            stmts.push_back(Declaration());

        Consume(TokenType::RIGHT_BRACE, "Expect '}' after block");
        std::unique_ptr<StmtBlock> stmt(new StmtBlock());
        stmt->stmts = std::move(stmts);
        return stmt;
    }

    // whileStmt -> "while" "(" expression ")" statement ;
    StmtPtr WhileStatement()
    {
        if (!Consume(TokenType::LEFT_PAREN, "Expect '(' after while"))
            return StmtPtr();
        ExprPtr condition = Expression();
        if (!Consume(TokenType::RIGHT_PAREN, "Expect ')' after while condition"))
            return StmtPtr();
        StmtPtr body = Statement();

        std::unique_ptr<StmtWhile> stmt(new StmtWhile());
        stmt->condition = std::move(condition);
        stmt->body = std::move(body);
        return stmt;
    }

    // ifStmt    -> "if" "(" expression ")" statement ( "else" statement )? ;
    StmtPtr IfStatement()
    {
        if (!Consume(TokenType::LEFT_PAREN, "Expect '(' after if"))
            return StmtPtr();
        ExprPtr condition = Expression();
        if (!Consume(TokenType::RIGHT_PAREN, "Expect ')' after if condition"))
            return StmtPtr();
        
        StmtPtr thenBranch = Statement();
        StmtPtr elseBranch;
        if (Match(TokenType::ELSE))
            elseBranch = Statement();

        std::unique_ptr<StmtIf> stmt(new StmtIf());
        stmt->condition = std::move(condition);
        stmt->thenBranch = std::move(thenBranch);
        stmt->elseBranch = std::move(elseBranch);
        return stmt;
    }

    // printStmt -> "print" expression ";"
    StmtPtr PrintStatement()
    {
        ExprPtr expr = Expression();
        if (!expr)
            return StmtPtr();
        if (!Consume(TokenType::SEMICOLON, "Expect ';' after expression"))
            return StmtPtr();
        
        std::unique_ptr<StmtPrint> stmt(new StmtPrint());
        stmt->expr = std::move(expr);
        return stmt;
    }

    // varDecl -> "var" IDENTIFIER ( "=" expression )? ";"
    StmtPtr VarDecl()
    {
        const Token* name = Consume(TokenType::IDENTIFIER, "Expected variable name");
        if (name == nullptr)
            return StmtPtr();

        ExprPtr initialiser;
        if (Match(TokenType::EQUAL))
            initialiser = Expression();

        if (!Consume(TokenType::SEMICOLON, "Expect ';' after variable declaration"))
            return StmtPtr();

        std::unique_ptr<StmtVar> stmt(new StmtVar());
        stmt->name = name;
        if (initialiser)
            stmt->initialiser = std::move(initialiser);
        return stmt;
    }

    // exprStmt -> expression ";"
    StmtPtr ExpressionStatement()
    {
        ExprPtr expr = Expression();
        if (!expr)
            return StmtPtr();
        if (!Consume(TokenType::SEMICOLON, "Expect ';' after expression"))
            return StmtPtr();
        
        std::unique_ptr<StmtExpression> stmt(new StmtExpression());
        stmt->expr = std::move(expr);
        return stmt;
    }

    // expression -> assignment
    ExprPtr Expression()
    {
        return Assignment();
    }

    // assignment -> identifier "=" assignment
    //             | logic_or
    ExprPtr Assignment()
    {
        ExprPtr expr = LogicOr();
        if (!expr)
            return ExprPtr();
        
        if (Match(TokenType::EQUAL))
        {
            const Token& equals = Previous();
            ExprPtr value = Assignment();
            if (!value)
                return ExprPtr();

            if (expr->type == ASTType::ExprVariable)
            {
                const Token* name = ((ExprVariable*)expr.get())->name;
                std::unique_ptr<ExprAssign> assignExpr(new ExprAssign());
                assignExpr->name = name;
                assignExpr->value = std::move(value);
                return assignExpr;
            }

            lox_error(equals, "Invalid assignment target");
            return ExprPtr();
        }
        return expr;
    }

    // logic_or -> logic_and ( "or" logic_or )*
    ExprPtr LogicOr()
    {
        ExprPtr expr = LogicAnd();
        if (!expr)
            return ExprPtr();

        while (Match(TokenType::OR))
        {
            const Token& op = Previous();
            ExprPtr right = LogicOr();
            if (!right)
                return ExprPtr();

            std::unique_ptr<ExprLogical> newExpr(new ExprLogical());
            newExpr->left = std::move(expr);
            newExpr->op = &op;
            newExpr->right = std::move(right);
            expr = std::move(newExpr);
        }

        return expr;
    }

    // logic_and -> equality ( "and" equality )*
    ExprPtr LogicAnd()
    {
        ExprPtr expr = Equality();
        if (!expr)
            return ExprPtr();

        while (Match(TokenType::AND))
        {
            const Token& op = Previous();
            ExprPtr right = Equality();
            if (!right)
                return ExprPtr();

            std::unique_ptr<ExprLogical> newExpr(new ExprLogical());
            newExpr->left = std::move(expr);
            newExpr->op = &op;
            newExpr->right = std::move(right);
            expr = std::move(newExpr);
        }

        return expr;
    }

    // equality -> comparison ( ( "!=" | "==" ) comparison )*
    ExprPtr Equality()
    {
        ExprPtr expr = Comparison();
        if (!expr)
            return ExprPtr();

        while (Match(TokenType::BANG_EQUAL) || Match(TokenType::EQUAL_EQUAL))
        {
            const Token& op = Previous();
            ExprPtr right = Comparison();
            if (!right)
                return ExprPtr();

            std::unique_ptr<ExprBinary> exprBin(new ExprBinary());
            exprBin->left = std::move(expr);
            exprBin->op = &op;
            exprBin->right = std::move(right);
            expr = std::move(exprBin);
        }

        return expr;
    }

    //comparison → addition ( ( ">" | ">=" | "<" | "<=" ) addition )*
    ExprPtr Comparison()
    {
        ExprPtr expr = Addition();
        if (!expr)
            return ExprPtr();

        while (Match(TokenType::GREATER) || Match(TokenType::GREATER_EQUAL) || Match(TokenType::LESS) || Match(TokenType::LESS_EQUAL))
        {
            const Token& op = Previous();
            ExprPtr right = Addition();
            if (!right)
                return ExprPtr();
            std::unique_ptr<ExprBinary> exprBin(new ExprBinary());
            exprBin->left = std::move(expr);
            exprBin->op = &op;
            exprBin->right = std::move(right);
            expr = std::move(exprBin);
        }

        return expr;
    }

    //addition       → multiplication ( ( "-" | "+" ) multiplication )*
    ExprPtr Addition()
    {
        ExprPtr expr = Multiplication();
        if (!expr)
            return ExprPtr();

        while (Match(TokenType::MINUS) || Match(TokenType::PLUS))
        {
            const Token& op = Previous();
            ExprPtr right = Multiplication();
            if (!right)
                return ExprPtr();
            std::unique_ptr<ExprBinary> exprBin(new ExprBinary());
            exprBin->left = std::move(expr);
            exprBin->op = &op;
            exprBin->right = std::move(right);
            expr = std::move(exprBin);
        }

        return expr;
    }

    //multiplication → unary ( ( "/" | "*" ) unary )*
    ExprPtr Multiplication()
    {
        ExprPtr expr = Unary();
        if (!expr)
            return ExprPtr();

        while (Match(TokenType::SLASH) || Match(TokenType::STAR))
        {
            const Token& op = Previous();
            ExprPtr right = Unary();
            if (!right)
                return ExprPtr();
            std::unique_ptr<ExprBinary> exprBin(new ExprBinary());
            exprBin->left = std::move(expr);
            exprBin->op = &op;
            exprBin->right = std::move(right);
            expr = std::move(exprBin);
        }

        return expr;
    }

    //unary          → ( "!" | "-" ) unary
    //               | call
    ExprPtr Unary()
    {
        if (Match(TokenType::BANG) || Match(TokenType::MINUS))
        {
            const Token& op = Previous();
            ExprPtr right = Unary();
            if (!right)
                return ExprPtr();
            std::unique_ptr<ExprUnary> exprUn(new ExprUnary());
            exprUn->op = &op;
            exprUn->right = std::move(right);
            return exprUn;
        }

        return Call();
    }

    ExprPtr FinishCall(ExprPtr& callee)
    {
        std::vector<ExprPtr> args;
        if (!Check(TokenType::RIGHT_PAREN))
        {
            do
            {
                args.push_back(Expression());
            }
            while (Match(TokenType::COMMA));
        }

        const Token* token = Consume(TokenType::RIGHT_PAREN, "Expect ')' after arguments");
        if (!token)
            return ExprPtr();

        std::unique_ptr<ExprCall> expr(new ExprCall());
        expr->callee = std::move(callee);
        expr->paren = token;
        expr->args = std::move(args);
        return expr;
    }

    //call -> primary ( "(" arguments? ")" )*
    ExprPtr Call()
    {
        ExprPtr expr = Primary();
        while (true)
        {
            if (Match(TokenType::LEFT_PAREN))
                expr = FinishCall(expr);
            else
                break;
        }
        return expr;
    }

    //primary        → NUMBER | STRING | "false" | "true" | "nil"
    //               | "(" expression ")" ;
    //               | IDENTIFIER
    ExprPtr Primary()
    {
        if (Match(TokenType::FALSE)) return Literal(Value(false));
        if (Match(TokenType::TRUE)) return Literal(Value(true));
        if (Match(TokenType::NIL)) return Literal(Value());

        if (Match(TokenType::NUMBER))
            return Literal(Value(Previous().numberLiteral));
        if (Match(TokenType::STRING))
            return Literal(Value(Previous().stringLiteral));

        if (Match(TokenType::LEFT_PAREN))
        {
            ExprPtr expr = Expression();
            if (!expr)
                return ExprPtr();
            if (!Consume(TokenType::RIGHT_PAREN, "Expect ')' after expression"))
                return ExprPtr();
            std::unique_ptr<ExprGrouping> exprGroup(new ExprGrouping());
            exprGroup->expr = std::move(expr);
            return exprGroup;
        }

        if (Match(TokenType::IDENTIFIER))
        {
            std::unique_ptr<ExprVariable> var(new ExprVariable());
            var->name = &Previous();
            return var;
        }

        lox_error(Peek(), "Expect expression");
        return ExprPtr();
    }

    ExprPtr Literal(const Value& value)
    {
        std::unique_ptr<ExprLiteral> expr(new ExprLiteral());
        expr->value = value;
        return expr;
    }
};

void parser_parse(const std::vector<Token>& tokens, std::vector<StmtPtr>& stmts)
{
    Parser parser(tokens);
    parser.Parse(stmts);
}
