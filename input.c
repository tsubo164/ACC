/*
#include <stdio.h>
*/

#include <stdio.h>
#include <math.h>

/*
void foo(float f)
{
    printf("%f\n", f);
}
*/

void assertf(float expected, float actual)
{
#if 0
    printf("expected: %f actual: %f\n", expected, actual);
    printf("expected - actual: %f\n", expected - actual);
    printf("fabsf: %f\n", fabsf(expected - actual));
#endif
    /* TODO use relative machine epsilon */
    if (fabsf(expected - actual) > 0.000001) {
        printf("error: expected: %f actual: %f\n", expected, actual);
        /*
        exit(1);
        */
    }
}

int main()
{
    float f = 3.14;
    float g = 0.5;

    //g = 3.1401;

    if (g < f)
        printf("g: %f is less than f: %f\n", g, f);
    else
        printf("g: %f is greater than f: %f\n", g, f);

    assertf(3.11, f);

    /*
    */
    printf("f - g: %f 2.64: %f\n", f - g, 2.64);
    //printf("fabsf(f - g): %f\n", fabsf(f - g));

    g = 3.1401;
    if (g > f)
        printf("g: %f is greater than f: %f\n", g, f);
    else
        printf("g: %f is less than f: %f\n", g, f);

    assertf(10, 11);
    assertf(10, 10.001);
    assertf(10.0, 10.0000);
    assertf(10.0002, 10.0002);

    return 42;
}
