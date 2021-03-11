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
#include <value.h>
#include <expression.h>
#include <expression_types.h>
struct lexcontext;
} //%code requires

%param {lexcontext& lex }
%code
{
struct lexcontext
{
    const char* cursor;
    const char* marker;
    yy::location loc;
};
namespace yy { conj_parser::symbol_type yylex(lexcontext& ctx); }
}

%token END 0
%token IDENTIFIER NUMCONST STRINGCONST RETURN "return"

%left  ';'
%left  ','
%left RETURN
%right '='
%left  '+' '-'
%left  '*' '/'
%left  '^'
%left  '('
%left  '['
%left  '{'


%type<Value> NUMCONST STRINGCONST IDENTIFIER
%type<Expression*> expression
%type<Constant*> lvalue
%type<StringConstant*> svalue
%type<Variable*> variable
%type<Assignment*> assignment
%type<Vector*> vector
%type<Vector*> vector-item
%type<Operation*> operation
%type<ReturnExpression*> return-block
%type<ExpressionBlock*> expression-block
%type<ExpressionBlock*> expression-item
%type<Function*> function
%type<FunctionCall*> function-call

%%

library: expression {Scope::scope->set_root_expression($1); }

expression-item: expression {$$ = new ExpressionBlock(); $$->add_expression($1); }
               | expression-item ';' expression {$$ = $1; $$->add_expression($3); }

expression-block: '{' expression-item ';' '}' {$$ = $2; }
                | '{' expression-item '}' {$$ = $2; }
                | '{' '}' {$$ = new ExpressionBlock(); } 

return-block: RETURN expression {$$ = new ReturnExpression($2); }

expression: expression-block { $$ = $1;}
          | return-block     { $$ = $1;}
          | assignment       { $$ = $1;}
          | function         { $$ = $1;}  
          | lvalue           { $$ = $1;}
          | svalue           { $$ = $1;}
          | variable         { $$ = $1;}
          | function-call    { $$ = $1;}
          | operation        { $$ = $1;}
          | vector           { $$ = $1;}

lvalue: NUMCONST        {$$ = new Constant($1); }

svalue: STRINGCONST     {$$ = new StringConstant($1);}

variable: IDENTIFIER {$$ = new Variable($1); }

assignment: variable '=' expression {$$ = new Assignment($1,$3); }

function: vector expression-block {$$ = new Function($1,$2); }

function-call: variable vector    {$$ = new FunctionCall($1,$2); }

operation: expression '+' expression     {$$ = new Operation($1,$3,op_sum); }
         | expression '-' expression     {$$ = new Operation($1,$3,op_sub); }
         | expression '*' expression     {$$ = new Operation($1,$3,op_mul); }
         | expression '/' expression     {$$ = new Operation($1,$3,op_div); }
         | expression '^' expression     {$$ = new Operation($1,$3,op_exp); }
         | expression '[' expression ']' {$$ = new Operation($1,$3,op_ref); }

vector-item: expression {$$ = new Vector(); $$->add_expression($1); }
           | vector-item ',' expression {$$ = $1; $$->add_expression($3); }

vector: '(' vector-item ')' {$$ = $2;}
      | '(' ')' {$$ = new Vector();}

%%

yy::conj_parser::symbol_type yy::yylex(lexcontext &ctx)
{
    const char* anchor = ctx.cursor;
    ctx.loc.step();
    auto s = [&](auto func, auto&&... params) { ctx.loc.columns(ctx.cursor - anchor); return func(params..., ctx.loc); };

        %{ /* Begin re2c lexer */
        re2c:yyfill:enable   = 0;
        re2c:define:YYCTYPE  = "char";
        re2c:define:YYCURSOR = "ctx.cursor";
        re2c:define:YYMARKER = "ctx.marker";
        
        // Keywords:
        "return"                { return s(conj_parser::make_RETURN); }
        /*
        "while" | "for"         { return s(conj_parser::make_WHILE); }
        "var"                   { return s(conj_parser::make_VAR); }
        "if"                    { return s(conj_parser::make_IF); } */
        
        // Identifiers:
        [a-zA-Z_] [a-zA-Z_0-9]* { return s(conj_parser::make_IDENTIFIER, std::string(anchor,ctx.cursor)); }
        
        // String and integer literals:
        "\"" [^"]* "\""         { return s(conj_parser::make_STRINGCONST, std::string(anchor+1, ctx.cursor-1)); }
        [0-9]+                  { return s(conj_parser::make_NUMCONST, std::stol(std::string(anchor,ctx.cursor))); } 
        [0-9]*"."[0-9]+         { return s(conj_parser::make_NUMCONST, std::stod(std::string(anchor,ctx.cursor))); } 
        [0-9]+"."[0-9]*         { return s(conj_parser::make_NUMCONST, std::stod(std::string(anchor,ctx.cursor))); } 
        // Whitespace and comments:
        "\000"                  { return s(conj_parser::make_END); }
        "\r\n" | [\r\n]         { ctx.loc.lines();   return yylex(ctx); }
        "//" [^\r\n]*           {                    return yylex(ctx); }
        [\t\v\b\f ]             { ctx.loc.columns(); return yylex(ctx); }
        
        // Multi-char operators and any other character (either an operator or an invalid symbol):
        /*
        "&&"                    { return s(conj_parser::make_AND); }
        "||"                    { return s(conj_parser::make_OR); }
        "++"                    { return s(conj_parser::make_PP); }
        "--"                    { return s(conj_parser::make_MM); }
        "!="                    { return s(conj_parser::make_NE); }
        "=="                    { return s(conj_parser::make_EQ); }
        "+="                    { return s(conj_parser::make_PL_EQ); }
        "-="                    { return s(conj_parser::make_MI_EQ); } */
        .                       { return s([](auto...s){return conj_parser::symbol_type(s...);}, conj_parser::token_type(ctx.cursor[-1]&0xFF)); } // Return that character 
        %} /* End lexer */
}

void yy::conj_parser::error(const location_type& l, const std::string& m)
{
    std::cerr << (l.begin.filename ? l.begin.filename->c_str() : "(undefined)");
    std::cerr << ':' << l.begin.line << ':' << l.begin.column << '-' << l.end.column << ": " << m << '\n';
}

#include <iostream>
#include <fstream>
int main(int argc, char** argv)
{
    std::string filename = argv[1];
    std::ifstream f(filename);
    std::string buffer(std::istreambuf_iterator<char>(f), {});

    lexcontext ctx;
    ctx.cursor = buffer.c_str();
    ctx.loc.begin.filename = &filename;
    ctx.loc.end.filename   = &filename;

    Scope scope;
    Scope::initialize_scope(&scope);

    yy::conj_parser parser(ctx);
    parser.parse();
    string prefix;
    debug_print_expression(scope.rootExpression,prefix);
    cout << scope.evaluate() << endl;
    string result;
    scope.rootExpression->print(result);
    cout << result << endl;
}
