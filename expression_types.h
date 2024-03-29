#pragma once
#include "global.h"
#include "expression.h"
#include "expression_util.h"
#include <sstream>
struct Constant : public Expression
{
    Value v;

    Constant(const Value& _v)
    {
        setType(ex_Constant);
        v = _v;
    }

    virtual Value i_evaluate() override { return v; }
    virtual void i_print(std::string &str)
    {
        str += v;
    }
};

struct StringConstant : public Expression
{
    Value str;
    Value finalStr;

    StringConstant(const Value& _str)
    {
        setType(ex_StringConstant);
        str = _str;
    }

    const std::string evalString()
    {
        std::stringstream ss(str);
        std::string token;
        std::string result = "";
        while (ss >> token)
        {
            if (token[0] == '$')
            {
                token.erase(0,1);
                Scope::scope->resolve(token)->print(result);
            }
            else
            result += token + " ";
        }
        return result;
    }
    virtual Value i_evaluate() override { finalStr.str = evalString(); return finalStr; }
    virtual void i_print(std::string &str)
    {
        str += finalStr;
    }
};
struct Variable : public Expression
{
    string name;
    bool represents_vector;

    Variable(const string& _name)
    { 
        setType(ex_Variable);
        name = _name; 
        represents_vector = false;
    }

    Expression* get() { return Scope::scope->resolve(name); }
    virtual Value i_evaluate() override { return get()->evaluate(); } 

    virtual void i_print(std::string& str)
    {
        if (represents_vector) str += "\\vec{";
        str += name;
        if (represents_vector) str += "}";
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
        if (assignment->getType() == ex_Vector) identifier->represents_vector = true;
    }

    virtual Value i_evaluate() override { 
        Expression* expr = Scope::scope->define(identifier->name,assignment); 
        return expr->evaluate();
    }

    virtual void i_print(std::string& str)
    {
        if (assignment->is_final())
        {
            identifier->print(str);
            str += " = ";
            latexize(assignment,str);
        }
        else if (assignment->getType() != ex_Function)
        {
            identifier->print(str);
            str += " = ";
            assignment->print(str);
        }
        else
        {
            identifier->print(str);
            assignment->print(str);
        }
        evaluate();
    }
};

struct Vector : public Expression
{
    vector<Expression*> variables;

    Vector() { setType(ex_Vector);  }

    virtual Value i_evaluate() override
    {
        if (variables.size() != 1)
        {
            Value values = variables[0]->evaluate()[0];
            for (size_t i = 1; i < variables.size(); i++)
            {
                values.push_back(variables[i]->evaluate()[0]);
            }
            return values;
        }

        return variables[0]->evaluate();
    }

    inline Expression* at(size_t index) { return variables[index]; }

    inline size_t size() const { return variables.size(); }

    inline void add_expression(Expression* expression)
    { 
        variables.emplace_back(expression); 
        dependency(expression);
    }

    void print_content(std::string& str)
    {
        for (size_t i = 0 ; i < variables.size(); i++)
        {
            variables[i]->print(str);
            if (i != variables.size() - 1) str += ",";
        }

    }
    virtual void i_print(std::string &str)
    {
        str += "(";
        print_content(str);
        str += ")";
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

    virtual Value i_evaluate() override 
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

    virtual void i_print(std::string& str)
    {
        if (op_type == op_div) str += "\\frac{";
        a->print(str);
        if (op_type == op_div) str += "}";
        switch(op_type)
        {
            case op_sum: str += " + "; break;
            case op_sub: str += " - "; break;
            case op_mul: str += " \\cdot "; break;
            case op_exp: str += " ^ "; break;
            case op_ref: str += "["; break;
        }
        
        if (op_type == op_div || op_type == op_exp) str += "{";
        b->print(str);
        if (op_type == op_div || op_type == op_exp) str += "}";

        if (op_type == op_ref) str += "]";
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

    virtual Value i_evaluate() override { return returnValue->evaluate(); }
    virtual void i_print(std::string & str)
    {
        str += "return ";
        returnValue->print(str);
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
    virtual Value i_evaluate() override
    {
        ++(*Scope::scope);      //Increase and decrease scope
        if (expectsParameters) initialize_body();
        Value v;
        for(int i = 0; i < expressions.size(); i++)
        {
            Expression* current = expressions[i];
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

    virtual void i_print(std::string & str)
    {
        for(Expression* expression : expressions)
        {
            expression->print(str);
            if (expression != expressions.back()) str += "\n";
        }
    }
};

struct Function : public Expression
{
    Vector* parameterVector;
    ExpressionBlock* expressionBlock;
    
    bool is_internal = false;

    Function()
    {
        setType(ex_Function);
    }
    Function(Vector* _parameterVector,ExpressionBlock* _expressionBlock) : parameterVector(_parameterVector), expressionBlock(_expressionBlock)
    { 
        setType(ex_Function);

        dependency(parameterVector);
        dependency(expressionBlock);
        expressionBlock->expectsParameters = true;
    }

    virtual Value i_evaluate() override { return 0; }

    virtual Value evaluate(Vector* valueVector)
    {
        expressionBlock->set_variable_vectors(parameterVector,valueVector);
        return expressionBlock->evaluate();
    }

    virtual void i_print(std::string & str)
    {
        parameterVector->print(str);
        str += " = ";
        expressionBlock->print(str);
    }
};

enum internalfunctionType
{
    fn_scalar,fn_vector,fn_expression
};

struct internalFunctionPtr
{
    using scalarFunctionPtr = double (*) (double);
    using vectorFunctionPtr = double (*) (Value);
    using expresionFunctionPtr = Expression* (*) (Vector*);

    scalarFunctionPtr scalarFunction;
    vectorFunctionPtr vectorFunction;
    expresionFunctionPtr expresionFunction;
    
    internalfunctionType type;

    internalFunctionPtr(const scalarFunctionPtr& _scalarFunction) : scalarFunction(_scalarFunction) { type = fn_scalar; }
    internalFunctionPtr(const vectorFunctionPtr& _vectorFunction) : vectorFunction(_vectorFunction) { type = fn_vector; } 
    internalFunctionPtr(const expresionFunctionPtr& _expresionFunction) : expresionFunction(_expresionFunction) { type = fn_expression; } 

    double get(double args)
    {
        return scalarFunction(args);
    }
    double get_vector(Value args)
    {
        return vectorFunction(args);
    }

    Expression* get_expr(Vector* args) { return expresionFunction(args); }
};
//This should be more efficient 
struct InternalFunction : public Function
{
    using internalFunction = internalFunctionPtr;
    
    std::string name;
    std::string prefix;
    std::string suffix;
    
    bool use_special_prefix = false;

    internalFunction functionPtr;

    InternalFunction(const std::string& _name,internalFunction _functionPtr) : name(_name), functionPtr(_functionPtr)
    {
        setType(ex_InternalFunction);
        is_internal = true;
    }
    virtual Value i_evaluate() override 
    {
        return 0;
    }

    virtual Value evaluate(Vector* valueVector) override
    {
        Value oldvalue;
        if (functionPtr.type != fn_expression) oldvalue = valueVector->at(0)->evaluate();
        Value result = oldvalue;
    
        switch(functionPtr.type)
        {
            case fn_vector: return functionPtr.get_vector(result);
            case fn_scalar: 
            default:

            for(int i = 0 ; i < oldvalue.size(); ++i)
            {
                result[i] = functionPtr.get(oldvalue[i]);
            }
            return result;
        }
    }

    void evaluate(Vector* valueVector, Expression* &expression)
    {
        expression = functionPtr.get_expr(valueVector);
    }

    bool is_expression_function()
    {
        return functionPtr.type == fn_expression;
    }

    static InternalFunction* registerInternalFunction(const std::string& name, internalFunction functionPtr)
    {
        return static_cast<InternalFunction*>(Scope::scope->define(name,new InternalFunction(name,functionPtr)));
    }

    void set_prefixes(const std::string& _prefix,const std::string& _suffix)
    {
        prefix = _prefix; suffix = _suffix;
        use_special_prefix = true;
    }
};
struct FunctionCall : public Expression
{
    Variable* functionIdentifier;
    Vector* valueVector;
    Expression* mutation;
    bool is_mutated = false;

    FunctionCall(Variable* _functionIdentifier,Vector* _valueVector) : functionIdentifier(_functionIdentifier), valueVector(_valueVector) 
    {
        setType(ex_FunctionCall);
        dependency(functionIdentifier);
        dependency(valueVector);
    }
    virtual Value i_evaluate()
    {
        if (is_mutated)
        {
            return mutation->evaluate();
        }
        Function* function = static_cast<Function*>(functionIdentifier->get());
        if (function->is_internal)
        {
            InternalFunction* intFunction = static_cast<InternalFunction*>(function);
            if (intFunction->is_expression_function())
            {
                intFunction->evaluate(valueVector,mutation);
                is_mutated = true;
                return i_evaluate();
            }
        }
        return function->evaluate(valueVector);
    }
    virtual void i_print(std::string & str)
    {
        if (is_mutated)
        {
            mutation->print(str);
            return;
        }
        Function* function = static_cast<Function*>(functionIdentifier->get());
        if (function->is_internal)
        {
            InternalFunction* internal = static_cast<InternalFunction*>(function);
            if (internal->use_special_prefix)
            {
                str += internal->prefix;
                valueVector->print_content(str);
                str += internal->suffix;
                return;
            }
        }
        functionIdentifier->print(str);
        valueVector->print(str);
    }
};


#define m_registerInternalFunction(x) InternalFunction::registerInternalFunction(#x,x);
#define m_registerInternalSpecialFunction(x,prefix,suffix) { auto* e = InternalFunction::registerInternalFunction(#x,x); e->set_prefixes(prefix,suffix); }
#define m_registerInternalConstant(x) Scope::scope->define(#x,new Constant(x));