#include "gcc_func.h"

int gcc_twice(int a)
{
    return 2 * a;
}

void gcc_add(char a, short b, int c, long d, long *out)
{
    *out = a + b + c + d;
}

int gcc_sum1234_mult_sum5678(
        int a1, int a2, int a3, int a4,
        int a5, int a6, int a7, int a8)
{
    return
        (a1 + a2 + a3 + a4) *
        (a5 + a6 + a7 + a8);
}
