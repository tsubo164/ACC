/*
#include <stdio.h>
*/

/* check omitted param */
int foo(int i)
{
    return 2 * i;
}

typedef int my_int;
typedef int (*foo_pointer)(int);

int main()
{
    my_int i = 121;
    foo_pointer fp = foo;

    return fp(i);
}
