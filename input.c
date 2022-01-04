/*
#include <stdio.h>
*/

struct point {
    int x, y;
    int bit : 3 + 2;
};

/*
struct point;
enum foo {
    A = 19 + 8, B, C
};
*/

struct point p;

struct point get_point()
{
    struct point p = {71, 92};
    return p;
}

int main()
{
    return 42;
    /*
    int i = 40, j = 2;

    return i + j;
    */
}
