int printf(char *s, int a, int b);
int exit(int code);

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

void set_count(int val)
{
    g_count = val;
}
