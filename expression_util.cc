#include "global.h"
#include "expression_util.h"
#include "expression_types.h"

void inlist(Expression* current,list<Expression*>& l)
{

    l.push_front(current);
    for(Expression* c : current->dependencies)
    {
        if (current->getType() == ex_FunctionCall && c->getType() == ex_Variable) continue;

        switch(c->getType())
        {
            case ex_Function:
            case ex_Constant: continue;
            default: inlist(c,l);
        }
    }
}
void latexize2(Expression* root,std::string& str,Expression* current = nullptr,bool print = true)
{
    if (current == nullptr)
    {
        current = root;
        root->print(str);
    }

    for(Expression* c : current->dependencies)
    {
        if (current->getType() == ex_FunctionCall && c->getType() == ex_Variable) continue;

        switch(c->getType())
        {
            case ex_Function:
            case ex_Constant: continue;
            default: latexize2(root,str,c,print);
        }
    }
    
    current->evaluate();
    if (print)
    {
        switch(current->getType())
        {
            case ex_Vector: break;
            default: 
                    str += " = ";
                    root->print(str);
        }
    }

}
void latexize(Expression* expression,std::string& str)
{
    latexize2(expression,str); return;
    list<Expression*> expressions;
    inlist(expression,expressions);

    for(auto it = expressions.begin(); it != expressions.end(); ++it)
    {
        expression->print(str);
        (*it)->evaluate();
        auto it2 = it;
        for(; (*it2)->getType() != (*it)->getType(); ++it2)
        {
            (*it2)->evaluate();
        }
        it = it2;
        str += " = ";
    }

    expression->print(str);
}
void debug_print_expression(Expression* root,const std::string& prefix)
{
    auto& dependencies = root->dependencies;
    for(auto it = dependencies.begin(); it != dependencies.end(); ++it)
    {
        Expression *c = *it;
        auto it2 = it; ++it2;

        cerr << prefix << (it2 != dependencies.end() ? "\u251c\u2500" : "\u2514\u2500") << "\u1405 " << literalType(c) << " " << c->is_final() << " ";
        switch(c->getType())
        {
            case ex_Constant:
            cerr << ((Constant*)c)->v;
            break;
            case ex_Variable:
            cerr << ((Variable*)c)->name;
            break;
            case ex_Operation:
            cerr << operationLiteral(c);
            break;
        }
        cerr << endl;
        if (c->dependencies.size() > 0)
        {
            string newprefix;
            if (it2 != dependencies.end()) newprefix = prefix + "\u2502 \t";
            else newprefix = prefix + " \t";
            debug_print_expression(c,newprefix);
        }
    }
}
