/*
#include <stdio.h>
*/

long foo(int a)
{
    return 2 * a;
}

int main()
{
    char c = 32;
    int a[4];

    a[2] = 42;

    if (a == &a[0])
        return 3;

    if (c < c)
        return 33;

    c = c > 0 ? 11 : 33;

    c = c | c;
    c = ! c;

    return foo(23);
    return sizeof a[2];
}
