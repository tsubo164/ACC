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

    p = &a;
    /*
    */
    return *(p + 3);
}
