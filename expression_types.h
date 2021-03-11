#pragma once
#include <global.h>
#include <expression.h>
#include <expression_util.h>

struct Constant : public Expression
{
    Value v;

    Constant(const Value& _v)
    {
        setType(ex_Constant);
        v = _v;
    }

    virtual Value evaluate() override { return v; }
    virtual void print(std::ostream &os)
    {
        os << v;
    }
};
struct Variable : public Expression
{
    string name;

    Variable(const string& _name)
    { 
        setType(ex_Variable);
        name = _name; 
    }

    Expression* get() { return Scope::scope->resolve(name); }
    virtual Value evaluate() override { return get()->evaluate(); } 

    virtual void print(std::ostream & os)
    {
        os << name;
    }
};
struct Assignment : public Expression 
{
    Variable* identifier;
    Expression* assignment;

    Assignment(Variable* _identifier, Expression* _assignment)
    {
        setType(ex_Assignment); 
        identifier = _identifier; 
        assignment = _assignment;
        dependency(identifier);
        dependency(assignment);
    }

    virtual Value evaluate() override { 
        Expression* expr = Scope::scope->define(identifier->name,assignment); 
        return expr->evaluate();
    }

    virtual void print(std::ostream & os)
    {
        if (assignment->getType() != ex_Function)
        {
            identifier->print(os);
            os << " = ";
            assignment->print(os);
        }
        else
        {
            identifier->print(os);
            assignment->print(os);
        }
    }
};

struct Vector : public Expression
{
    vector<Expression*> variables;

    Vector() { setType(ex_Vector);  }

    virtual Value evaluate() override
    {
        vector<double> values(variables.size());
        for (size_t i = 0; i < variables.size(); i++)
        {
            values[i] = variables[i]->evaluate()[0];
        }
        return Value(values);
    }

    inline Expression* at(size_t index) { return variables[index]; }

    inline size_t size() const { return variables.size(); }

    inline void add_expression(Expression* expression)
    { 
        variables.emplace_back(expression); 
        dependency(expression);
    }
    virtual void print(std::ostream & os)
    {
        os << "(";
        for (size_t i = 0 ; i < variables.size(); i++)
        {
            variables[i]->print(os);
            if (i != variables.size() - 1) os << ",";
        }
        os << ")";
    }
};

#define OPERATION_TYPE_ENUM(o) \
    o(op_sum) \
    o(op_sub) \
    o(op_mul) \
    o(op_div) \
    o(op_exp) \
    o(op_ref)

#define o(n) n,
enum OperationType { OPERATION_TYPE_ENUM(o) };
#undef o

#define o(n) #n,
static const char* OperationTypeLiterals[] = { OPERATION_TYPE_ENUM(o) };
#undef o

#define operationLiteral(e) ( OperationTypeLiterals[(static_cast<Operation*>(e))->op_type] )

struct Operation : public Expression
{
    Expression *a,*b;
    OperationType op_type;
    Operation(Expression *_a,Expression *_b,OperationType _op_type) : a(_a), b(_b),op_type(_op_type) 
    {
        setType(ex_Operation);
        dependency(a);
        dependency(b);
    }

    virtual Value evaluate() override 
    { 

        #define case_operation(type,operatort) case type: return a->evaluate() operatort b->evaluate();
        switch(op_type)
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

    virtual void print(std::ostream & os)
    {
        if (op_type == op_div) os << "\\frac{";
        a->print(os);
        if (op_type == op_div) os << "}";
        switch(op_type)
        {
            case op_sum: os << " + "; break;
            case op_sub: os << " - "; break;
            case op_mul: os << " \\cdot "; break;
            case op_exp: os << " ^ "; break;
            case op_ref: os << "["; break;
        }
        
        if (op_type == op_div) os << "{";
        b->print(os);
        if (op_type == op_div) os << "}";

        if (op_type == op_ref) os << "]";
    }
};

struct ReturnExpression : public Expression
{
    Expression* returnValue;
    ReturnExpression(Expression* _returnValue) : returnValue(_returnValue) 
    {
        setType(ex_ReturnExpression);
        dependency(returnValue);
    }

    virtual Value evaluate() override { return returnValue->evaluate(); }
    virtual void print(std::ostream & os)
    {
        os << "return ";
        returnValue->print(os);
    }
};

struct ExpressionBlock : public Expression
{
    vector<Expression*> expressions;
    Vector *parameterVector, *valueVector;
    bool expectsParameters = false;

    ExpressionBlock() { setType(ex_ExpressionBlock); }

    void set_variable_vectors(Vector* _parameterVector,Vector* _valueVector)
    {
        parameterVector = _parameterVector;
        valueVector = _valueVector;
    }
    void initialize_body()
    {
        assert(parameterVector);
        assert(valueVector);
        assert(parameterVector->size() == valueVector->size());
        int m = parameterVector->size();
        for(int i = 0; i < m; i++)
        {
            Scope::scope->define(static_cast<Variable*>(parameterVector->at(i))->name,valueVector->at(i));
        }
    }
    virtual Value evaluate() override
    {
        ++(*Scope::scope);      //Increase and decrease scope
        if (expectsParameters) initialize_body();
        Value v;
        for(int i = 0; i < expressions.size(); i++)
        {
            Expression* current = expressions[i];
            latexize(current);
            v = current->evaluate();
            if (getType() == ex_ReturnExpression) break;
        }
        --(*Scope::scope);
        parameterVector = nullptr;
        valueVector = nullptr;
        return v;
    }

    void add_expression(Expression* expression)
    {
        expressions.emplace_back(expression);
        dependency(expression);
    }

    virtual void print(std::ostream & os)
    {
        for(Expression* expression : expressions)
        {
            expression->print(os);
            if (expression != expressions.back())os << endl;
        }
    }
};

struct Function : public Expression
{
    Vector* parameterVector;
    ExpressionBlock* expressionBlock;

    Function(Vector* _parameterVector,ExpressionBlock* _expressionBlock) : parameterVector(_parameterVector), expressionBlock(_expressionBlock)
    { 
        setType(ex_Function);
        dependency(parameterVector);
        dependency(expressionBlock);

        expressionBlock->expectsParameters = true;
    }

    virtual Value evaluate() override { return 0; }
    Value evaluate(Vector* valueVector)
    {
        expressionBlock->set_variable_vectors(parameterVector,valueVector);
        return expressionBlock->evaluate();
    }

    virtual void print(std::ostream & os)
    {
        parameterVector->print(os);
        os << " = ";
        expressionBlock->print(os);
    }
};

struct FunctionCall : public Expression
{
    Variable* functionIdentifier;
    Vector* valueVector;

    FunctionCall(Variable* _functionIdentifier,Vector* _valueVector) : functionIdentifier(_functionIdentifier), valueVector(_valueVector) 
    {
        setType(ex_FunctionCall);
        dependency(functionIdentifier);
        dependency(valueVector);
    }
    virtual Value evaluate()
    {
        Function* function = static_cast<Function*>(functionIdentifier->get());
        return function->evaluate(valueVector);
    }
    virtual void print(std::ostream & os)
    {
        functionIdentifier->print(os);
        valueVector->print(os);
    }
};
