%skeleton "lalr1.cc"
%language "c++"
%define api.parser.class {conj_parser}
%define api.token.constructor
%define api.value.type variant
%define parse.assert
%define parse.error verbose
%locations   // <--
%code requires
{
#include "value.h"
#include "expression.h"
} //%code requires

%code
{
namespace yy { conj_parser::symbol_type yylex(); }
}

%token END 0
%token IDENTIFIER NUMCONST STRINGCONST RETURN "return"

%left  ';'
%left  ','
%right '='
%left  '+' '-'
%left  '*' '/'
%left  '^'
%left  '('
%left  '['
%left  '{'
%right  '}'
%right  ')'


%type<Value> NUMCONST STRINGCONST
%type<Expression*> expression
%type<Constant*> lvalue
%type<Variable*> IDENTIFIER
%type<Assignment*> assignment
%type<Vector*> vector
%type<Operation*> operation
%type<ReturnExpression*> return-block
%type<ExpressionBlock*> expression-block
%type<Function*> function
%type<FunctionCall*> function-call

%%

library: expression {Scope::ctx->set_root_expression($1); }

expression-block: '{' {$$ = new ExpressionBlock(); }
                |  expression-block ';' expression { $$ = $1; $$->add_expression($3); }
                |  expression-block '}' {$$ = $1; }

return-block: RETURN expression {$$ = new ReturnExpression($2); }

expression: expression-block { $$ = $1;}
          | return-block     { $$ = $1;}
          | assignment       { $$ = $1;}
          | function         { $$ = $1;}  
          | lvalue           { $$ = $1;}
          | IDENTIFIER       { $$ = $1;}
          | function-call    { $$ = $1;}
          | operation        { $$ = $1;}
          | vector           { $$ = $1;}
//          | IDENTIFIER '[' expression ']'

assignment: IDENTIFIER '=' expression {$$ = new Assignment($1,$3); }

lvalue: NUMCONST        {$$ = new Constant($1); }
      | STRINGCONST     {$$ = new Constant($1); }

function: vector expression-block {$$ = new Function($1,$2); }

function-call: IDENTIFIER vector    {$$ = new FunctionCall($1,$2); }

operation: expression '+' expression {$$ = new Operation($1,$3,op_sum); }
         | expression '-' expression {$$ = new Operation($1,$3,op_sub); }
         | expression '*' expression {$$ = new Operation($1,$3,op_mul); }
         | expression '/' expression {$$ = new Operation($1,$3,op_div); }
         | expression '^' expression {$$ = new Operation($1,$3,op_exp); }
         | expression '[' expression ']' {$$ = new Operation($1,$3,op_ref); }

vector: '(' {$$ = new Vector(); }
      | vector ',' expression {$$ = $1; $$->add_expression($3); } 
      | vector ')' {$$ = $1; }

%%

#include <iostream>
int main(int argc, char** argv)
{
    Scope ctx;
    Scope::initialize_ctx(&ctx);
}
