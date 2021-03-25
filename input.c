#include "input.h"

/*
int foo()
{
    return;
}

void bar()
{
    return 42;
}
*/

int *baz()
{
    int l = 234;
    return &l;
}

int main()
{
    int i = 42;

    return i;
}
