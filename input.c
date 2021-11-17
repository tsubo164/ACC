/*
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
*/

typedef struct point {
    int x, y, z;
} point;

int foo(const char *s, int *i, point p)
{
    return *i + p.x;
}

int main()
{
    point p = {11, 22, 33};
    const char *s = "Heloo, world\n";
    int i = 42;

    return foo(s, &i, p);
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
