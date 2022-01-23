/*
#include <stdio.h>
*/

    /*
struct foo {
    int x, y;
    char c, d;
};

int add(int x, int y, int z)
{
    return "foo";
    return x + y + z;
}
    */

int num()
{
    return 23012;
}

int main()
{
    int (*fp)() = num;

    return sizeof(int);
}

void copyi(int *d, const int *s)
{
    *d = *s;
}

struct data_type {
    int i;
};

void copy_data_type(struct data_type *dst, const struct data_type *src)
{
    *dst = *src;
}
