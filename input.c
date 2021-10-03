/*
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
*/

typedef struct point {
    int x, y;
} Point;

Point get_point()
{
    Point pt = {111, 222};
    return pt;
}

int main()
{
    Point pt = get_point();
    return pt.y;
}
