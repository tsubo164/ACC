/*
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
*/

struct opecode {
    int a;
};

void foo(const struct opecode *op, const int *nchars)
{
    *nchars = 3;
}

int main()
{
    const int i = 3;
    int a[4];

    i = 9;
    a[2] = 19;

    return a[2];
}

/*
void foo();
static int foo(void);

int main()
{
    int a = bar();

    return foo();
}

static int foo(void)
{
    return 42;
}
*/
