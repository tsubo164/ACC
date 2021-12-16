/*
*/
#include <stdio.h>

struct foo {
    signed int a : 5;
    signed int b : 17;
    signed int c : 10;
    /*
    signed int a : 5;
    signed int b : 29;
    signed int c : 10;
    */
};

int main()
{
    struct foo f;

    f.a = 3;

    printf("sizeof f: %ld\n", sizeof f);

    return f.a;
}
