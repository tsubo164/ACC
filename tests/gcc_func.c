#include "gcc_func.h"

int gcc_twice(int a)
{
    return 2 * a;
}

void gcc_add(char a, short b, int c, long d, long *out)
{
    *out = a + b + c + d;
}
