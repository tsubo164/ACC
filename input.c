/*
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
*/

typedef struct point {
    int x, y, z;
} Point;

Point get_point()
{
    Point pt = {111, 222, 55};
    return pt;
}

int add(int x, int y)
{
    return x + y;
}

int main()
{
    Point pt = get_point();
    return pt.z;
}
