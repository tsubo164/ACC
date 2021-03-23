#include "input.h"

/*
int inc(int *i)
{
    return ++*i;
}
*/
int foo(void)
{
    return 42;
}

int add(int x, int y, int z)
{
    return x + y + z;
}

int main()
{
    int i = add(11, 14, 29);
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
        long a;// = 233334012321;
        //unsigned short b;// = 2334;
        long b;// = 2334;
        long c;

        //assert(1503, a % b);

        a = 123534233349;
        //a = 349;
        b = 337;
        //assert(-124, a % b);
        //
        c = a % b;
        //c = a - b;

        //printf("c = %d\n", c);
    */

    return i;
}
