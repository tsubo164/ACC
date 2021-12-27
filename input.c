/*
#include <stdio.h>
*/

int foo()
{
    return 42;
}

int main()
{
    int (*fp)() = foo;
    /*
    int (*fp)();
    */
    fp = foo;

    return foo();
}
