/*
#include <stdio.h>
*/

struct foo {
    int x, y, z;
};

int bar(char c);

int main()
{
    int a;
    if (a < 3)
        goto final;

final:
    return sizeof(struct foo );
}
