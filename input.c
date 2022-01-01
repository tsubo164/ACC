/*
#include <stdio.h>
*/

int num(void)
{
    return 112;
}

long foo(int)
{
    return 23;
}

int (*fp)() = num;


int main()
{
    fp = foo;
    fp = num;
    return fp();
}
