#pragma once
#include <global.h>
#include <value.h>
#include <scope.h>
#include <map>
#include <stack>
#include <list>
#include <cassert>

#define ExpressionType_d(o) \
    o(ex_Constant) \
    o(ex_StringConstant) \
    o(ex_Variable) \
    o(ex_Assignment) \
    o(ex_Vector) \
    o(ex_Operation) \
    o(ex_ReturnExpression) \
    o(ex_ExpressionBlock) \
    o(ex_Function) \
    o(ex_FunctionCall)

#define o(n) n,
enum ExpressionType { ExpressionType_d(o) };
#undef o

#define o(n) #n, 
static const char* ExpressionType_literals[] = { ExpressionType_d(o) };
#undef o

#define literalType(e) ExpressionType_literals[e->getType()]

struct Expression
{
    Expression() { }
    Expression(const Expression& other) = delete;
    
    void setType(ExpressionType _type) {  type = _type;  }
    ExpressionType getType() const {return type; }

    virtual Value evaluate() = 0;

    bool is_final() const 
    {
        for(Expression* expr: dependencies) if (!expr->is_final()) return false;

        switch  (type)
        {
            case ex_Assignment:
            return false;
            default: 
            return true;
        }
    }

    virtual void print(std::string& str) { }
    private:
    ExpressionType type;


    #ifdef DEBUG
    public:
    list<Expression*> dependencies;
    #endif

};

#ifdef DEBUG
#define dependency(x) dependencies.push_back(x)
#endif
