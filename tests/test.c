int printf(const char *format, ...);
int exit(int code);

double fabs(double x);
float fabsf(float x);

#include "test.h"

/* for external global variable tests */
int g_count = 0;

void assert(int expected, int actual)
{
    if (expected != actual) {
        printf("error: expected: %d actual: %d\n", expected, actual);
        exit(1);
    }
}

void assertl(long expected, long actual)
{
    if (expected != actual) {
        printf("error: expected: %ld actual: %ld\n", expected, actual);
        exit(1);
    }
}

void assertf(float expected, float actual)
{
    /* TODO use relative machine epsilon */
    if (fabsf(expected - actual) > 0.000001) {
        printf("error: expected: %f actual: %f\n", expected, actual);
        exit(1);
    }
}

void set_count(int val)
{
    g_count = val;
}
