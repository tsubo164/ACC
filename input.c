/*
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
*/

union foo {
    int i;
    long l;
    const char *s;
    struct point p;
};

union var;

int main()
{
    union foo f;
    union var *v;
    int *i;


    v->i = 23;
    f.a = 3;

    return i.member;
}
