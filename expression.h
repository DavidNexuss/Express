#pragma once
#include <value.h>
#include <map>
#include <stack>
#include <list>
#include <cassert>

enum ExpressionType
{
    ex_Constant = 0,
    ex_Variable,
    ex_Assignment,
    ex_Vector,
    ex_Operation, 
    ex_ReturnExpression,
    ex_ExpressionBlock,
    ex_Function,
    ex_FunctionCall
};

struct Expression
{
    ExpressionType type;
    virtual Value evaluate() = 0;
};

struct Scope
{

    static Scope* scope;
    
    static void initialize_scope(Scope* _scope) { scope = _scope; }

    Expression* rootExpression;
    vector<map<string,Expression*>> variableStack;
    int currentScope = 0;


    Scope() { variableStack.emplace_back(); }

    void operator ++()
    {
        if (currentScope == variableStack.size() - 1) variableStack.emplace_back();
        currentScope++;
        variableStack[currentScope].clear();
    }

    void operator --()
    {
        currentScope--;
    }

    Expression* resolve(const string& name) { return variableStack[currentScope][name]; }
    Expression* define(const string& name,Expression* expression) { return variableStack[currentScope][name] = expression; }

    void set_root_expression(Expression* expression) { rootExpression = expression; }

    Value evaluate() { return rootExpression->evaluate(); }
};
Scope* Scope::scope = nullptr;

struct Constant : public Expression
{
    Value v;

    Constant() { type = ex_Constant; }
    Constant(const Value& _v) : v(_v) { Constant();}

    template <typename ... T>
    Constant(const T& ... args) : v(args...) { Constant();}

    virtual Value evaluate() override { return v; }
};
struct Variable : public Expression
{
    string name;

    Variable() { type = ex_Variable; }
    Variable(const string& _name) : name(_name) { Variable(); }

    Expression* get() { return Scope::scope->resolve(name); }

    virtual Value evaluate() override { return get()->evaluate(); } 

};
struct Assignment : public Expression 
{
    Variable* identifier;
    Expression* assignment;

    Assignment() { type = ex_Assignment; }
    Assignment(Variable* _identifier, Expression* _assignment) : identifier(_identifier), assignment(_assignment) { Assignment(); }

    virtual Value evaluate() override { 
        Expression* expr = Scope::scope->define(identifier->name,assignment); 
        if (expr->type == ex_Function) return 0;
        return expr->evaluate();
    }
};

struct Vector : public Expression
{
    vector<Expression*> variables;

    Vector() { type = ex_Vector; }
    virtual Value evaluate() override
    {
        vector<double> values(variables.size());
        for (size_t i = 0; i < variables.size(); i++)
        {
            values[i] = variables[i]->evaluate()[0];
        }
        return Value(values);
    }

    Expression* at(size_t index)
    {
        return variables[index];
    }

    inline size_t size() const { return variables.size(); }

    inline void add_expression(Expression* expression) { variables.emplace_back(expression); } 
};

enum OperationType
{
    op_sum = 0,
    op_sub,
    op_mul,
    op_div,
    op_exp,
    op_ref
};


struct Operation : public Expression
{
    Expression *a,*b;
    OperationType op_type;
    Operation(Expression *_a,Expression *_b,OperationType _op_type) : a(_a), b(_b),op_type(_op_type) 
    {
        type = ex_Operation;
    }

    virtual Value evaluate() override 
    { 

        #define case_operation(type,operatort) case type: return a->evaluate() operatort b->evaluate();
        switch(type)
        {
            case_operation(op_sum,+);
            case_operation(op_sub,-);
            case_operation(op_mul,*);
            case_operation(op_div,/);
            case_operation(op_exp,^);
            case op_ref:
                return a->evaluate()[b->evaluate()[0]];
        }
        #undef case_operation

        throw new std::runtime_error("Invalid operation type");
    }
};

struct ReturnExpression : public Expression
{
    Expression* returnValue;
    ReturnExpression(Expression* _returnValue) : returnValue(_returnValue) 
    {
        type = ex_ReturnExpression;
    }

    virtual Value evaluate() override { return returnValue->evaluate(); }
};

struct ExpressionBlock : public Expression
{
    vector<Expression*> expressions;

    ExpressionBlock()
    {
        type = ex_ExpressionBlock;
    }
    ExpressionBlock(const vector<Expression*>& _expressions) : expressions(_expressions) 
    { 
        ExpressionBlock();
    }

    virtual void initialize_body() { }
    virtual Value evaluate() override
    {
        ++(*Scope::scope);      //Increase and decrease scope
        initialize_body();
        Value v;
        for(int i = 0; i < expressions.size(); i++)
        {
            Expression* current = expressions[i];
            v = current->evaluate();
            if (current->type == ex_ReturnExpression) break;
        }
        --(*Scope::scope);
        return v;
    }

    void add_expression(Expression* expression)
    {
        expressions.emplace_back(expression);
    }
};

struct Function : public ExpressionBlock
{
    Vector* parameterVector;
    Vector* valueVector;

    Function(Vector* _parameterVector,const vector<Expression*>& _expressions) : ExpressionBlock(_expressions), parameterVector(_parameterVector) 
    { 
        type = ex_Function;
    } 
    Function(Vector* _parameterVector,ExpressionBlock* expression_block)
    {
        Function(_parameterVector,expression_block->expressions);
        delete expression_block;
    }
    virtual void initialize_body()
    {
        assert(parameterVector->size() == valueVector->size());
        int m = parameterVector->size();
        for(int i = 0; i < m; i++)
        {
            Scope::scope->define(parameterVector->at(i)->evaluate().as_string(),valueVector->at(i));
        }
    }
};

struct FunctionCall : public Expression
{
    Variable* functionIdentifier;
    Vector* valueVector;

    FunctionCall()
    {
        type = ex_FunctionCall;
    }

    FunctionCall(Variable* _functionIdentifier,Vector* _valueVector) : functionIdentifier(_functionIdentifier), valueVector(_valueVector) 
    {
        FunctionCall();
    }
    virtual Value evaluate()
    {
        Function* function = static_cast<Function*>(functionIdentifier->get());
        function->valueVector = valueVector;
        return function->evaluate();
    }
};
