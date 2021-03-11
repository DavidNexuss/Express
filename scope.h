#pragma once
#include <global.h>
#include <value.h>
#include <vector>
#include <map>
#include <string>

struct Expression;
struct Scope
{
    static Scope* scope;
    static void initialize_scope(Scope* _scope) { scope = _scope; }

    Expression* rootExpression;
    std::vector<std::map<std::string,Expression*>> variableStack;
    int currentScope;

    Scope();

    void operator ++();
    void operator --();

    Expression* resolve(const std::string& name);
    Expression* define(const std::string& name,Expression* expression);

    void set_root_expression(Expression* expression);

    Value evaluate();
};