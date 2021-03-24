#include "input.h"

/*
int inc(int *i)
{
    return ++*i;
}
int foo(void)
{
    return 42;
}

int add(int x, int y, int z)
{
    return x + y + z;
}
*/

int bar(long l, int *i)
{
    return ++*i + l;
}

/*
*/
struct point {
    int *x, *y;
};

int main()
{
    /*
    int i = add(11, 14, 29);
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
    int a = 17;
    struct point pt = {1, 112};
    //struct point pt = {&a, &a};

    //i = bar('a', &i);
    //i = bar(&i, 'a');


    return *pt.x;
}
