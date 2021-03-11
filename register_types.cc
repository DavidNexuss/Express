#include <expression_types.h>

#define internalFunctions(o) \
    o(sin) o(cos)
void registerInternalFunctions()
{
    internalFunctions(registerInternalFunction);
}
