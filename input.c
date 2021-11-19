/*
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
*/

#include <stdio.h>
static const char *SI__[] = {"sil", "si", "esi", "rsi"};
static const char *DI__[] = {"dil", "di", "edi", "rdi"};
static const char **ARG_REG__[] = {DI__, SI__};

/* TODO need support */
/*
char c[10] = "FOO";
*/

int main()
{
    /* TODO need support */
    /*
    char a[] = "array";
    */
    printf(">>  [%s]\n", ARG_REG__[0][2]);
    printf(">>  sizeof [%d]\n", sizeof ARG_REG__);

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
