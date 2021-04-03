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

void add_point(struct point *out, const struct point *p, const struct point *q)
{
    out->x = p->x + q->x;
    out->y = p->y + q->y;
}

int sub(int x, int y)
{
    return x - y;
}

int main()
{
    struct point p = {42, 71};
    int x = 11, y = 3;

    return sub(x + y, getx(&p));
    /*
    struct point p = {42, 71};
    struct point q = {13, 49};
    struct point x;

    add_point(&x, &p, &q);

    return x.x;
    */
}
