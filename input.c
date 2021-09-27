/*
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
*/

/*
typedef struct point {
    int x, y, z;
} Point;
*/

/*
int get_x(Point p)
{
    return p.z;
}

int main()
{
    Point p = {11, 22, 33};

    return get_x(p);
}
*/

int sum1234_mult_sum5678(
        int a1, int a2, int a3, int a4,
        int a5, int a6, int a7, int a8)
{
    return
        (a1 + a2 + a3 + a4) *
        (a5 + a6 + a7 + a8);
}

int add(int x, int y, int z)
{
    return x - y + z;
}

int main()
{
    /*
    return add(11, 22, 33);
    */
    int a = sum1234_mult_sum5678(1, 2, 3, 4, 5, 6, 7, 8);

    return a - 100;
}
