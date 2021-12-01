/*
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
*/

union foo {
    int i;
    long l;
    const char *s;
};

int main()
{
    union foo f = {42, 12321130239, (void*)0};

    return f.i;
}
