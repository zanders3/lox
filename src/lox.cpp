#include "lox.h"
#include "scanner.h"
#include "parser.h"
#include "interpreter/interpreter.h"

bool g_hadError = false;

void lox_run(const std::shared_ptr<Environment>& env, const char* source, int sourceLen)
{
    g_hadError = false;

    std::vector<Token> tokens;
    scanner_scan(source, sourceLen, tokens);

    std::vector<std::unique_ptr<Stmt>> stmts;
    parser_parse(tokens, stmts);

    Interpreter interpreter(env);
    interpreter.ExecuteBlock(stmts);
    printf("\n");
}

void lox_error(int line, const char* message)
{
    g_hadError = true;
    printf("[line %d] Error %s\n", line, message);
}

void lox_error(const Token& token, const char* message)
{
    if (token.type == TokenType::END)
        printf("[line %d] Error %s at end\n", token.line, message);
    else
        printf("[line %d] Error %s at %s\n", token.line, message, token.lexeme);
}
