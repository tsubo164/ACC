/*
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
*/
#include <stdio.h>

int main()
{
    int c = 32;
    printf("\\%03o", c);
    printf("\"\\n\"\n");
    printf("================\n");
    printf("[%c]\n", '\"');
    printf("[%c]\n", '"');
    printf("================\n");
    printf("test'test\n");
    printf("test\'test\n");
    printf("================\n");
    printf("test\test\n");
    printf("test\\test\n");
    return c;
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
