/*
#include <stdio.h>
*/

#include <stdio.h>

/*
float num(float f)
{
    return 3 * f;
}
*/
float foo(float f)
{
    return f;
}

int main() {
    /*
    int i = 3;
    */
    float f;

    f = 3;

    /*
    f = i;
    printf("%f\n", f);
    f = num(f);
    foo(f);
    */

    return 42;
}
