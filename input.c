/*
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
*/

enum {
    A = 1 << 0,
    B = 1 << 1,
    C = 1 << 2
};

int main()
{
    int a = B | C;

    return a;
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
