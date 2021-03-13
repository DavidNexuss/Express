#include "global.h"
#include "expression_util.h"
#include "expression_types.h"

void latexize(Expression* expression)
{
}
void debug_print_expression(Expression* root,const std::string& prefix)
{
    auto& dependencies = root->dependencies;
    for(auto it = dependencies.begin(); it != dependencies.end(); ++it)
    {
        Expression *c = *it;
        auto it2 = it; ++it2;

        cerr << prefix << (it2 != dependencies.end() ? "\u251c\u2500" : "\u2514\u2500") << "\u1405 " << literalType(c) << " ";
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
