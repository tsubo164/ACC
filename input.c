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
    char name[8];


    v->i = 23;
    f.a = 3;

    return i.member;
}
