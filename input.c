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
    /*
    Point pt = {111, 222, 33};
    */
    Point pt = {111, 222};
    return pt;
}

int add(int x, int y)
{
    return x + y;
}

int main()
{
    /*
    Point pt;
    pt = get_point();
    return pt.y;
    */
    int a = add(3, 7);
    return a;
    /*
    */
}
