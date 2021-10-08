/*
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
*/

typedef struct point {
    int x, y, z;
} Point;

void copy_point(Point *dst, const Point *src)
{
    if (!dst || !src || dst == src)
        return;
    *dst = *src;
}

int main()
{
    Point p = {11, 22, 33};
    Point q;

    copy_point(&q, &p);

    return q.y;
}
