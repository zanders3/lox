#pragma once
#include <memory>

struct Token;
class Environment;

void lox_run(const std::shared_ptr<Environment>& env, const char* source, int sourceLen);
void lox_error(const Token& token, const char* message);
void lox_error(int line, const char* message);
