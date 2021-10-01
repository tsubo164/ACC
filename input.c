/*
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
*/

typedef struct coord {
    long x, y, z;
} Coord;

long coord_x(Coord c)
{
    return c.x;
}

long coord_y(Coord c)
{
    return c.y;
}

long coord_z(Coord c)
{
    return c.z;
}

int main()
{
    Coord c = {111, 222, 199};
    return coord_z(c);
}
