/*
#include <stdio.h>
*/

#include <stdio.h>

void foo(float f)
{
    printf("%f\n", f);
}
/*
*/

int main()
{
    /*
    int i = 3;
    float f, g;
    */
    double g;

    g = 3.14;
    foo(g);
    /*
    f = 19;

    foo(f + i);
    */
    /*
    printf("%g\n", f);
    */

    return 42;
}
