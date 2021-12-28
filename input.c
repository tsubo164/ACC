/*
#include <stdio.h>
*/

int foo()
{
    return 24;
}

int main()
{
    int (*fp)() = foo;
    /*
    int (*fp)();
    fp = foo;
    fp();
    */

    return fp();
}
