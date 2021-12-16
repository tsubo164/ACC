/*
#include <stdio.h>
*/

struct foo {
    signed int a : 5;
    signed int b : 17;
    signed int c : 10;
};

int main()
{
    struct foo f;

    f.a = 3;
    f.b = 31;
    f.c = 42;

    return f.c;
}
