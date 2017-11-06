#pragma once
#include "scanner.h"
#include "value.h"
#include <array>
#include <memory>

#define AST(rettype, parent, name, vars, ret) struct parent##name;
#include "astdefs.h"
#undef AST

struct Visitor
{
    virtual ~Visitor();
    #define AST(rettype, parent, name, vars, ret) virtual rettype Visit##name(const parent##name& val) = 0;
    #include "astdefs.h"
    #undef AST
};

enum class ASTType
{
    #define AST(rettype, parent, name, vars, ret) parent##name,
    #include "astdefs.h"
    #undef AST
};

struct Expr
{
    ASTType type;

    virtual ~Expr();
    virtual Value Visit(Visitor& visitor) const = 0;
};

struct Stmt
{
    ASTType type;

    virtual ~Stmt();
    virtual void Visit(Visitor& visitor) const = 0;
};

#define AST(rettype, parent, name, vars, ret) struct parent##name : public parent {\
    parent##name() { type = ASTType::parent##name; }\
    vars virtual rettype Visit(Visitor& visitor) const override\
    {\
        ret visitor.Visit##name(*this);\
    }\
};
#include "astdefs.h"
#undef AST