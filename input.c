/*
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
*/

typedef struct point {
    int x, y, z, w;
} Point;

int get_x(Point p)
{
    return p.x;
}

int get_y(Point p)
{
    return p.y;
}

int get_z(Point p)
{
    return p.z;
}

int get_w(Point p)
{
    return p.w;
}

int main()
{
    Point p = {11, 22, 33, 44};
    return get_w(p);
}
