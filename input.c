/*
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
*/

typedef struct coord {
    long x, y, z;
} Coord;

Coord get_coord()
{
    Coord c = {111, 222, 55};
    return c;
}

int main()
{
    Coord c = get_coord();
    return c.z;
}
