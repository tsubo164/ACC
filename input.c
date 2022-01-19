/*
#include <stdio.h>
*/

struct foo {
    int x, y, z;
};

int bar(char c);

int main()
{
    int a = 10;

    goto start;

start:
    if (a < 3)
        goto final;

final:
    return sizeof(struct foo );

    goto bar;

start:
    return 0;
    /*
    */
foo:
    return 0;

    goto start;
}
