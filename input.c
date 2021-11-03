/*
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
*/
#define NULL ((void*)0)
#define NEW_(k) new_node(k, NULL, NULL)
NEW_(NOD_NUM)

#define ADD(a, b) (a + b)
ADD(2, 3)
/*
int main()
{
    enum {
        A = 1 << 0,
        B = 1 << 1,
        C = 1 << 2
    };

    int i = A;
    i |= B;

    return i;
}
*/

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
