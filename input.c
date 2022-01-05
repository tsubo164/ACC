/*
#include <stdio.h>
*/

struct point {
    int x, y;
    /*
    int bit : 3 + 2;
    */
};

int foo = 3;
/*
struct point;
enum foo {
    A = 19 + 8, B, C
};

struct point p;

struct point get_point()
{
    struct point p = {71, 92};
    return p;
}
*/
int num()
{
    return 12;
}


int main()
{
    int (*fp)() = num;
    int i = 19;
    struct point p = {71, 29, 111};
    struct point o = {17};

    char i;
    long q;
    short z;

    return p.x + p.y + i + foo + fp() + q;
    /*
    int i = 40, j = 2;

    return i + j;
    */
}
