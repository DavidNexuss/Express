#include <expression_types.h>

#define internalFunctions(o) \
    o(sin) o(cos) o(tan) o(exp) o(exp2) o(log) o(log2) o(log10) o(sqrt) o(cbrt) o(ceil) o(floor) o(abs)
#define internalConstants(o) \
    o(M_PI)
void registerInternalFunctions()
{
    internalFunctions(registerInternalFunction);
    internalConstants(registerInternalConstant);
}
