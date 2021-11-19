/*
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
*/
#include <stdio.h>

typedef struct coord {
    long x, y, z;
} Coord;

Coord ret_struct_by_val_with_struct_param(Coord cd, const char *s, int id)
{
    Coord c = cd;
    c.x += id;
    c.y += id;
    c.z += id;

    printf("s: [%s]: (%ld, %ld, %ld)\n", s, c.x, c.y, c.z);
    return c;
}

int main()
{

    Coord c = {11, 22, 33};

    c = ret_struct_by_val_with_struct_param(c, "foo", 19);

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
