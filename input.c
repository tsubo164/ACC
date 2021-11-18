/*
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
*/
#include <stdio.h>

typedef struct coord {
    long x, y, z;
} Coord;

Coord ret_struct_by_val(const char *s, int id)
{
    Coord c = {11, 22, 33};
    c.x += id;

    printf("s: [%s]\n", s);
    return c;
}

int main()
{
    Coord c = ret_struct_by_val("foo", 19);

    return c.x;
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
