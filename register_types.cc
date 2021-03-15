#include "expression_types.h"

Expression* solve(Vector* v)
{
   //Manual interpretation
}
#define internalFunctions(o) \
    o(sin) o(cos) o(tan) o(ceil) o(floor) o(solve)

#define internalSpecialFunctions(o) \
    o(vsum,"\\sum{","}") o(vprod,"\\prod{","}") \
    o(sqrt,"\\sqrt{","}") o(abs,"\\abs{","}") \
    o(log,"\\log{","}") o(log2,"\\log_{2}{","}") o(log10,"\\log_{10}{","}") \
    o(exp,"e^{","}") o(exp2,"e^{2\\cdot","}") \
    o(abs,"\\left|","\\right|")

#define internalConstants(o) \
    o(M_PI)
void registerInternalFunctions()
{
    internalFunctions(m_registerInternalFunction);
    internalSpecialFunctions(m_registerInternalSpecialFunction);
    internalConstants(m_registerInternalConstant);
}
