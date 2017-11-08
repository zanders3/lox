#pragma once
#include <vector>
#include <memory>
#include "ast.h"

struct Token;

bool parser_parse(const std::vector<Token>& tokens, StmtPtrList& stmts);
