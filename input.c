/*
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
*/

static int foo(void);

int main()
{
    int a = bar();

    return foo();
}

static int foo(void)
{
    return 42;
}
