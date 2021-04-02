/*
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
*/

struct point {
    int x, y;
};

int getx(struct point *p)
{
    return p->x;
}

int sub(int x, int y)
{
    return x - y;
}

int main()
{
    struct point p = {42, 71};
    int x = 11, y = 3;

    return getx(&p) + sub(x, y);
}
