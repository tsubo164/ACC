/*
*/
#include <stdio.h>

struct foo {
    signed int a : 11;
    signed int : 4;
    signed int b : 18;
    signed int c : 20;
    signed int : 0;
    signed int d: 7;
};

int main()
{
    struct foo f;

    f.c = -7;

    printf("%d\n", f.c);

    return sizeof f;
}
