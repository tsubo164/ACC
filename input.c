/*
#include <stdio.h>
*/

typedef struct foo {
    int x;
    char y;
} Foo;

long foo(char a)
{
    return 2 * a;
}

int main()
{
    Foo f = {11, 22};

    return foo(23);
}
