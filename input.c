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

int main()
{
    int a[4] = {11, 22, 33, 44};

    int *p = /*(int *)*/ a;

    int *q = 0;
    /*
    p = 0;
    */
    return *(p + 3);
}
