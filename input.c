#include "input.h"

/*
int inc(int *i)
{
    return ++*i;
}
*/
int foo(void)
{
    return 42;
}

int add(int x, int y, int z)
{
    return x + y + z;
}

int main()
{
    int i = add();
    /*
    int i = foo();
    int i = foo(11);
    int i = add(11, 14);
    int i = add(11, 14);
    int i = 13;

    i = inc(&i);
    */

    return i;
}
