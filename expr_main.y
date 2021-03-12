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
    registerInternalFunctions();

    yy::conj_parser parser(ctx);
    parser.parse();
    string prefix;
    debug_print_expression(scope.rootExpression,prefix);
    cout << scope.evaluate() << endl;
    string result;
    scope.rootExpression->print(result);
    cout << result << endl;
}
