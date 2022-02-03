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
int add(int a, int b)
{
    return a + b;
}

int main()
{
    int i = 3;
    float f;

    f = 19;

    foo(f + i);
    /*
    printf("%g\n", f);
    */

    return add(31, 11);
}
