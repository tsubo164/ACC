/*
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
*/
#include <stdio.h>

long bar(long l)
{
    return l;
}

int main()
{
    long l = 0;
    int i = -32;

    l = i;
    printf("----> %ld\n", l);

    l = bar(i);
    printf("----> %ld\n", l);

    return 42;
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
