/*
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
*/
typedef struct Coord {
    long x, y, z;
} Coord;

Coord get_coord(long l)
{
    Coord c = {7, -19, 0};
    c.z = l;
    return c;
}

long some_calc(long a, long b, Coord coord)
{
    return a + b + coord.x;
}

int main()
{
    long l = 23;

    l = some_calc(9, 17, get_coord(55));

    return l;
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
