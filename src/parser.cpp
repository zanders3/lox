#include "parser.h"
#include "lox.h"
#include "scanner.h"
#include "ast.h"

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
    void Parse(std::vector<std::unique_ptr<Stmt>>& stmts)
    {
        while (!IsAtEnd())
        {
            std::unique_ptr<Stmt> stmt(Declaration());
            if (stmt)
                stmts.push_back(std::move(stmt));
        }
    }

    //declaration -> funcDecl | varDecl | statement
    std::unique_ptr<Stmt> Declaration()
    {
        std::unique_ptr<Stmt> stmt;
        //if (Match(TokenType::FUN)) stmt = Function();
        if (Match(TokenType::VAR)) stmt = VarDecl();
        else stmt = Statement();

        if (!stmt)
            Synchronize();

        return stmt;
    }

    // statement -> exprStmt | forStmt | ifStmt | printStmt | returnStmt | whileStmt | block
    std::unique_ptr<Stmt> Statement()
    {
        if (Match(TokenType::PRINT)) return PrintStatement();

        return ExpressionStatement();
    }

    // printStmt -> "print" expression ";"
    std::unique_ptr<Stmt> PrintStatement()
    {
        std::unique_ptr<Expr> expr = Expression();
        if (!expr)
            return std::unique_ptr<Stmt>();
        if (!Consume(TokenType::SEMICOLON, "Expect ';' after expression"))
            return std::unique_ptr<Stmt>();
        
        std::unique_ptr<StmtPrint> stmt(new StmtPrint());
        stmt->expr = std::move(expr);
        return stmt;
    }

    // varDecl -> "var" IDENTIFIER ( "=" expression )? ";"
    std::unique_ptr<Stmt> VarDecl()
    {
        const Token* name = Consume(TokenType::IDENTIFIER, "Expected variable name");
        if (name == nullptr)
            return std::unique_ptr<Stmt>();

        std::unique_ptr<Expr> initializer;
        if (Match(TokenType::EQUAL))
            initializer = Expression();

        if (!Consume(TokenType::SEMICOLON, "Expect ';' after variable declaration"))
            return std::unique_ptr<Stmt>();

        std::unique_ptr<StmtVar> stmt(new StmtVar());
        stmt->name = name;
        if (initializer)
            stmt->initializer = std::move(initializer);
        return stmt;
    }

    // exprStmt -> expression ";"
    std::unique_ptr<Stmt> ExpressionStatement()
    {
        std::unique_ptr<Expr> expr = Expression();
        if (!expr)
            return std::unique_ptr<Stmt>();
        if (!Consume(TokenType::SEMICOLON, "Expect ';' after expression"))
            return std::unique_ptr<Stmt>();
        
        std::unique_ptr<StmtExpression> stmt(new StmtExpression());
        stmt->expr = std::move(expr);
        return stmt;
    }

    // expression -> assignment
    std::unique_ptr<Expr> Expression()
    {
        return Assignment();
    }

    // assignment -> identifier "=" assignment
    //             | logic_or
    std::unique_ptr<Expr> Assignment()
    {
        std::unique_ptr<Expr> expr = LogicOr();
        if (!expr)
            return std::unique_ptr<Expr>();
        
        if (Match(TokenType::EQUAL))
        {
            const Token& equals = Previous();
            std::unique_ptr<Expr> value = Assignment();
            if (!value)
                return std::unique_ptr<Expr>();

            if (expr->type == ASTType::ExprVariable)
            {
                const Token* name = ((ExprVariable*)expr.get())->name;
                std::unique_ptr<ExprAssign> assignExpr(new ExprAssign());
                assignExpr->name = name;
                assignExpr->value = std::move(value);
                return assignExpr;
            }

            lox_error(equals, "Invalid assignment target");
            return nullptr;
        }
        return expr;
    }

    // logic_or -> logic_and ( "or" logic_or )*
    std::unique_ptr<Expr> LogicOr()
    {
        std::unique_ptr<Expr> expr = LogicAnd();
        if (!expr)
            return std::unique_ptr<Expr>();

        while (Match(TokenType::OR))
        {
            const Token& op = Previous();
            std::unique_ptr<Expr> right = LogicOr();
            if (!right)
                return std::unique_ptr<Expr>();

            std::unique_ptr<ExprLogical> newExpr(new ExprLogical());
            newExpr->left = std::move(expr);
            newExpr->op = &op;
            newExpr->right = std::move(right);
            expr = std::move(newExpr);
        }

        return expr;
    }

    // logic_and -> equality ( "and" equality )*
    std::unique_ptr<Expr> LogicAnd()
    {
        std::unique_ptr<Expr> expr = Equality();
        if (!expr)
            return std::unique_ptr<Expr>();

        while (Match(TokenType::AND))
        {
            const Token& op = Previous();
            std::unique_ptr<Expr> right = Equality();
            if (!right)
                return std::unique_ptr<Expr>();

            std::unique_ptr<ExprLogical> newExpr(new ExprLogical());
            newExpr->left = std::move(expr);
            newExpr->op = &op;
            newExpr->right = std::move(right);
            expr = std::move(newExpr);
        }

        return expr;
    }

    // equality -> comparison ( ( "!=" | "==" ) comparison )*
    std::unique_ptr<Expr> Equality()
    {
        std::unique_ptr<Expr> expr = Comparison();
        if (!expr)
            return std::unique_ptr<Expr>();

        while (Match(TokenType::BANG_EQUAL) || Match(TokenType::EQUAL_EQUAL))
        {
            const Token& op = Previous();
            std::unique_ptr<Expr> right = Comparison();
            if (!right)
                return std::unique_ptr<Expr>();

            std::unique_ptr<ExprBinary> exprBin(new ExprBinary());
            exprBin->left = std::move(expr);
            exprBin->op = &op;
            exprBin->right = std::move(right);
            expr = std::move(exprBin);
        }

        return expr;
    }

    //comparison → addition ( ( ">" | ">=" | "<" | "<=" ) addition )*
    std::unique_ptr<Expr> Comparison()
    {
        std::unique_ptr<Expr> expr = Addition();
        if (expr == nullptr)
            return nullptr;

        while (Match(TokenType::GREATER) || Match(TokenType::GREATER_EQUAL) || Match(TokenType::LESS) || Match(TokenType::LESS_EQUAL))
        {
            const Token& op = Previous();
            std::unique_ptr<Expr> right = Addition();
            if (!right)
                return std::unique_ptr<Expr>();
            std::unique_ptr<ExprBinary> exprBin(new ExprBinary());
            exprBin->left = std::move(expr);
            exprBin->op = &op;
            exprBin->right = std::move(right);
            expr = std::move(exprBin);
        }

        return expr;
    }

    //addition       → multiplication ( ( "-" | "+" ) multiplication )*
    std::unique_ptr<Expr> Addition()
    {
        std::unique_ptr<Expr> expr = Multiplication();
        if (expr == nullptr)
            return nullptr;

        while (Match(TokenType::MINUS) || Match(TokenType::PLUS))
        {
            const Token& op = Previous();
            std::unique_ptr<Expr> right = Multiplication();
            if (right == nullptr)
                return std::unique_ptr<Expr>();
            std::unique_ptr<ExprBinary> exprBin(new ExprBinary());
            exprBin->left = std::move(expr);
            exprBin->op = &op;
            exprBin->right = std::move(right);
            expr = std::move(exprBin);
        }

        return expr;
    }

    //multiplication → unary ( ( "/" | "*" ) unary )*
    std::unique_ptr<Expr> Multiplication()
    {
        std::unique_ptr<Expr> expr = Unary();
        if (expr == nullptr)
            return nullptr;

        while (Match(TokenType::SLASH) || Match(TokenType::STAR))
        {
            const Token& op = Previous();
            std::unique_ptr<Expr> right = Unary();
            if (right == nullptr)
                return std::unique_ptr<Expr>();
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
    std::unique_ptr<Expr> Unary()
    {
        if (Match(TokenType::BANG) || Match(TokenType::MINUS))
        {
            const Token& op = Previous();
            std::unique_ptr<Expr> right = Unary();
            if (!right)
                return std::unique_ptr<Expr>();
            std::unique_ptr<ExprUnary> exprUn(new ExprUnary());
            exprUn->op = &op;
            exprUn->right = std::move(right);
            return exprUn;
        }

        return Call();
    }

    //call -> primary ( "(" arguments? ")" )*
    std::unique_ptr<Expr> Call()
    {
        return Primary();
        //TODO: function call
    }

    //primary        → NUMBER | STRING | "false" | "true" | "nil"
    //               | "(" expression ")" ;
    //               | IDENTIFIER
    std::unique_ptr<Expr> Primary()
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
            std::unique_ptr<Expr> expr = Expression();
            if (!expr)
                return std::unique_ptr<Expr>();
            if (!Consume(TokenType::RIGHT_PAREN, "Expect ')' after expression"))
                return std::unique_ptr<Expr>();
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
        return nullptr;
    }

    std::unique_ptr<Expr> Literal(const Value& value)
    {
        std::unique_ptr<ExprLiteral> expr(new ExprLiteral());
        expr->value = value;
        return expr;
    }
};

void parser_parse(const std::vector<Token>& tokens, std::vector<std::unique_ptr<Stmt>>& stmts)
{
    Parser parser(tokens);
    parser.Parse(stmts);
}
