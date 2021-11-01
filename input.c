/*
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
*/

int main()
{
    int a = 15;

    a %= 4;

    a = 24145;
    a %= 1233;

    return a;
}

/*
void foo();
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
*/
