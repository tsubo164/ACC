/*
#include <stdio.h>
*/

int num(void)
{
    return 112;
}

/* check omitted param */
int foo(int i)
{
    return 23;
}

long bar()
{
    return 23;
}

int (*fp)(void) = num;
static int (*foop)(int);

int main()
{
    /*
    fp = foo;
    fp = bar;
    */
    /*
    */
    fp = num;
    foop = foo;
    return fp();
}
