/*
#include <stdio.h>
*/

#include <stdio.h>

void foo(float f)
{
    printf("%f\n", f);
}

void bar(int i)
{
    printf("%d\n", i);
}
/*
*/

int main()
{
    /*
    int i = 3;
    float f, g;
    float g;

    g = 3.14;
    */
    double g = 3.14;
    foo(g);
    /*

    bar(g);
    */
    /*
    f = 19;

    foo(f + i);
    */
    /*
    printf("%g\n", f);
    */

    return 42;
}
