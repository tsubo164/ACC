/*
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
*/

struct foo {
    int i;
};

struct tree_context__ {
    /*
    struct foo *f;
    */
    const struct foo *f;
    int array_length;
};

void check_initializer__(void)
{
    struct tree_context__ new_ctx;

    new_ctx.array_length = 0;
}

int main()
{
    return 42;
}
