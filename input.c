/*
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
*/
#include <stdio.h>

typedef struct coord {
    long x, y, z;
} Coord;

Coord verts[] = {
    {111, 222, 333},
    {444, 555, 666},
    {777, 888, 999}
};

struct data_spec {
    const char *suffix;
    const char *sizename;
};

const struct data_spec data_spec_table[] = {
    {"b", "byte"},
    {"w", "word"},
    {"l", "long"},
    {"q", "quad"}
};

const char *A__[]  = {"al",  "ax", "eax", "rax"};

char *str = "Hello";
int *p = ((void*)1);
/* TODO need support */
/*
char c[10] = "FOO";
*/

int main()
{
    char *s = "local";
    /* TODO need support */
    /*
    char a[] = "array";
    return 42 + a[0];
    return 42;
    */

    printf(">> [%s]\n", data_spec_table[1].sizename);
    printf(">> [%s]\n", A__[2]);
    return s[3];
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
