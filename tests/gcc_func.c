#include "gcc_func.h"

int gcc_twice(int a)
{
    return 2 * a;
}

void gcc_add(char a, short b, int c, long d, long *out)
{
    *out = a + b + c + d;
}

int gcc_sum1234_mult_sum5678(
        int a1, int a2, int a3, int a4,
        int a5, int a6, int a7, int a8)
{
    return
        (a1 + a2 + a3 + a4) *
        (a5 + a6 + a7 + a8);
}

int gcc_get_x(point p)
{
    return p.x;
}

int gcc_get_y(point p)
{
    return p.y;
}

point gcc_get_point(void)
{
    point p = {71, 92};
    return p;
}

int gcc_get_x4(vec v)
{
    return v.x;
}

int gcc_get_y4(vec v)
{
    return v.y;
}

int gcc_get_z4(vec v)
{
    return v.z;
}

int gcc_get_w4(vec v)
{
    return v.w;
}

vec gcc_get_vec(void)
{
    vec v = {1301, 223922, -3973};
    return v;
}

long gcc_coord_x(Coord c)
{
    return c.x;
}

long gcc_coord_y(Coord c)
{
    return c.y;
}

long gcc_coord_z(Coord c)
{
    return c.z;
}
