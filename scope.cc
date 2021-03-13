#include "scope.h"
#include "expression.h"
Scope::Scope() 
{
    currentScope = 0; 
    variableStack.emplace_back(); 
}

void Scope::operator ++()
{
    if (currentScope == variableStack.size() - 1) variableStack.emplace_back();
    currentScope++;
    variableStack[currentScope].clear();
}

void Scope::operator --() { currentScope--; }

Expression* Scope::resolve(const string& name) 
{ 
    int i = currentScope;
    while(i >= 0)
    {
        auto it = variableStack[i].find(name);
        if (it != variableStack[i].end()) return it->second;
        i--;
    }
    
    cerr << "Variable " << name << " not found in any scope" << endl;
    throw std::runtime_error("Variable not found");
}
Expression* Scope::define(const string& name,Expression* expression) 
{ 
    #ifdef DEBUG
    cerr << "Scope: " << currentScope << " : Variable definition " << name << " as " << literalType(expression) << endl;
    #endif
    return variableStack[currentScope][name] = expression; 
}

void Scope::set_root_expression(Expression* expression)
{ 
    rootExpression = expression; 
}
Value Scope::evaluate() 
{ 
    return rootExpression->evaluate(); 
}

Scope* Scope::scope = nullptr;
