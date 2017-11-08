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

    //declaration -> classDecl | funcDecl | varDecl | statement
    StmtPtr Declaration()
    {
        StmtPtr stmt;
        if (Match(TokenType::CLASS)) stmt = Class();
        else if (Match(TokenType::FUN)) stmt = Function();
        else if (Match(TokenType::VAR)) stmt = VarDecl();
        else stmt = Statement();

        if (!stmt)
            Synchronize();

        return stmt;
    }

    // classDecl -> "class" IDENTIFIER "{" function* "}"
    StmtPtr Class()
    {
        const Token* name = Consume(TokenType::IDENTIFIER, "Expected class name");
        if (!name)
            return StmtPtr();
        if (!Consume(TokenType::LEFT_BRACE, "Expect '{' before class body"))
            return StmtPtr();

        StmtFunctionPtrList methods;
        while (!Check(TokenType::RIGHT_BRACE) && !IsAtEnd())
            methods.push_back(Function());

        Consume(TokenType::RIGHT_BRACE, "Expected '}' after class body");
        return StmtPtr(new StmtClass(name, std::move(methods)));
    }

    // funcDecl -> "fun" function
    // function -> IDENTIFIER "(" parameters? ")" block
    StmtFunctionPtr Function()
    {
        const Token* name = Consume(TokenType::IDENTIFIER, "Expected function name");
        if (!name)
            return StmtFunctionPtr();
        if (!Consume(TokenType::LEFT_PAREN, "Expected '(' after function name"))
            return StmtFunctionPtr();

        std::vector<const Token*> params;
        if (!Check(TokenType::RIGHT_PAREN))
        {
            do
            {
                const Token* param = Consume(TokenType::IDENTIFIER, "Expected parameter name");
                if (!param)
                    return StmtFunctionPtr();
                params.push_back(param);
            }
            while (Match(TokenType::COMMA));
        }
        if (!Consume(TokenType::RIGHT_PAREN, "Expected ')' after parameters"))
            return StmtFunctionPtr();
        if (!Consume(TokenType::LEFT_BRACE, "Expected '{' before function body"))
            return StmtFunctionPtr();
        std::vector<StmtPtr> body;
        ParseBlock(body);

        return StmtFunctionPtr(new StmtFunction(name, std::move(params), std::move(body)));
    }

    // statement -> exprStmt | forStmt | ifStmt | printStmt | returnStmt | whileStmt | block
    StmtPtr Statement()
    {
        if (Match(TokenType::PRINT)) return PrintStatement();
        if (Match(TokenType::IF)) return IfStatement();
        if (Match(TokenType::WHILE)) return WhileStatement();
        if (Match(TokenType::LEFT_BRACE)) return BlockStatement();
        if (Match(TokenType::FOR)) return ForStatement();
        if (Match(TokenType::RETURN)) return ReturnStatement();

        return ExpressionStatement();
    }

    // returnStmt -> "return" expression? ";" 
    StmtPtr ReturnStatement()
    {
        const Token* keyword = &Previous();
        ExprPtr value;
        if (!Check(TokenType::SEMICOLON)) {
            value = Expression();
        }
        Consume(TokenType::SEMICOLON, "Expect ';' after return value");

        return StmtPtr(new StmtReturn(keyword, std::move(value)));
    }

    // forStmt -> "for" "(" (varDecl | exprStmt | ";") expression? ";" expression? ")" statement
    StmtPtr ForStatement()
    {
        if (!Consume(TokenType::LEFT_PAREN, "Expect '(' after 'for'"))
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
            StmtPtrList block;
            block.push_back(std::move(body));
            block.push_back(StmtPtr(new StmtExpression(std::move(increment))));
            body = StmtPtr(new StmtBlock(std::move(block)));
        }

        if (!condition)
            condition = ExprPtr(new ExprLiteral(true));

        body = StmtPtr(new StmtWhile(std::move(condition), std::move(body)));

        if (initialiser)
        {
            StmtPtrList block;
            block.push_back(std::move(initialiser));
            block.push_back(std::move(body));
            body = StmtPtr(new StmtBlock(std::move(block)));
        }
        
        return body;
    }

    void ParseBlock(std::vector<StmtPtr>& stmts)
    {
        while (!Check(TokenType::RIGHT_BRACE) && !IsAtEnd())
            stmts.push_back(Declaration());

        Consume(TokenType::RIGHT_BRACE, "Expect '}' after block");
    }

    // block -> declaration*
    StmtPtr BlockStatement()
    {
        std::vector<StmtPtr> stmts;
        ParseBlock(stmts);
        return StmtPtr(new StmtBlock(std::move(stmts)));
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

        return StmtPtr(new StmtWhile(std::move(condition), std::move(body)));
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

        return StmtPtr(new StmtIf(std::move(condition), std::move(thenBranch), std::move(elseBranch)));
    }

    // printStmt -> "print" expression ";"
    StmtPtr PrintStatement()
    {
        ExprPtr expr = Expression();
        if (!expr)
            return StmtPtr();
        if (!Consume(TokenType::SEMICOLON, "Expect ';' after expression"))
            return StmtPtr();
        
        return StmtPtr(new StmtPrint(std::move(expr)));
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

        return StmtPtr(new StmtVar(name, std::move(initialiser)));
    }

    // exprStmt -> expression ";"
    StmtPtr ExpressionStatement()
    {
        ExprPtr expr = Expression();
        if (!expr)
            return StmtPtr();
        if (!Consume(TokenType::SEMICOLON, "Expect ';' after expression"))
            return StmtPtr();
        return StmtPtr(new StmtExpression(std::move(expr)));
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

            if (expr->type == ExprType::Variable)
            {
                const Token* name = static_cast<const ExprVariable*>(expr.get())->name;
                return ExprPtr(new ExprAssign(name, std::move(value)));
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
            expr = ExprPtr(new ExprLogical(std::move(expr), &op, std::move(right)));
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
            expr = ExprPtr(new ExprLogical(std::move(expr), &op, std::move(right)));
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
            expr = ExprPtr(new ExprBinary(std::move(expr), &op, std::move(right)));
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
            expr = ExprPtr(new ExprBinary(std::move(expr), &op, std::move(right)));
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
            expr = ExprPtr(new ExprBinary(std::move(expr), &op, std::move(right)));
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
            expr = ExprPtr(new ExprBinary(std::move(expr), &op, std::move(right)));
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
            return ExprPtr(new ExprUnary(&op, std::move(right)));
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
        return ExprPtr(new ExprCall(std::move(callee), token, std::move(args)));
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
        if (Match(TokenType::FALSE)) return ExprPtr(new ExprLiteral(false));
        if (Match(TokenType::TRUE)) return ExprPtr(new ExprLiteral(true));
        if (Match(TokenType::NIL)) return ExprPtr(new ExprLiteral());

        if (Match(TokenType::NUMBER))
            return ExprPtr(new ExprLiteral(Previous().numberLiteral));
        if (Match(TokenType::STRING))
            return ExprPtr(new ExprLiteral(Previous().stringLiteral));

        if (Match(TokenType::LEFT_PAREN))
        {
            ExprPtr expr = Expression();
            if (!expr)
                return ExprPtr();
            if (!Consume(TokenType::RIGHT_PAREN, "Expect ')' after expression"))
                return ExprPtr();
            return ExprPtr(new ExprGrouping(std::move(expr)));
        }

        if (Match(TokenType::IDENTIFIER))
            return ExprPtr(new ExprVariable(&Previous()));

        lox_error(Peek(), "Expect expression");
        return ExprPtr();
    }
};

bool parser_parse(const std::vector<Token>& tokens, std::vector<StmtPtr>& stmts)
{
    Parser parser(tokens);
    parser.Parse(stmts);

    for (const StmtPtr& stmt : stmts)
        if (!stmt)
            return false;
    return true;
}
