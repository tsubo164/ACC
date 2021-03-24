#include "input.h"

struct vec {
    int x, y;
    char c;
    long l[4];
} v = {123, 2312, 'a', {1,2}};
/*
int inc(int *i)
{
    return ++*i;
}
int foo(void)
{
    return 42;
}
*/

/*
int add(int x, int y, int z)
{
    return x + y + z;
}

int bar(long l, int *i)
{
    return ++*i + l;
}
*/

struct point {
    int *x, *y;
};
int a = 17;

int main()
{
    /*
    int i = add(11, 14, &a, 29);
    int j = bar(&a);

    return i + j;
    */

    /*
    int i = foo();
    int i = foo(11);
    int i = -27 % 7;
    printf("i: %d\n", i);
    int i = foo();
    int i = add();
    int i = add(11, 14);
    int i = 13;

    i = inc(&i);
    */

    //int i = 13;
    //int a = &i;
    //struct point pt = {1, 112};
    //struct point pt = {&a, &a, &a};
    //struct point pt = {&a, 12, &a};
    struct point pt = {&a, &a};

    return *pt.x;
    /*
    //i = bar('a', &i);
    //i = bar(&i, 'a');
    //int *p[2] = {12, 34};
    //int *p[2] = {&a, 34};
    int *p[2] = {34, &a, &a};

    //int i = &a;
    //int *i = 1;


    return *p[1];
    */
}
