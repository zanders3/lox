#include "lox.h"
#include "scanner.h"
#include "parser.h"
#include "resolver.h"
#include "interpreter/interpreter.h"

void lox_run(const std::shared_ptr<Environment>& env, const char* source, int sourceLen)
{
    std::vector<Token> tokens;
    scanner_scan(source, sourceLen, tokens);

    StmtPtrList stmts;
    if (!parser_parse(tokens, stmts))
        return;

    if (!resolver_resolve(stmts))
        return;

    Interpreter interpreter(env);
    interpreter.ExecuteBlock(stmts);
    printf("\n");
}

void lox_error(int line, const char* message)
{
    printf("[line %d] Error %s\n", line, message);
}

void lox_error(const Token& token, const char* message)
{
    if (token.type == TokenType::END)
        printf("[line %d] Error %s at end\n", token.line, message);
    else
        printf("[line %d] Error %s at %s\n", token.line, message, token.lexeme);
}
