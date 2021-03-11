int printf(char *s, int a, int b);
int exit(int code);

#include "test.h"

/* for external global variable tests */
int g_count = 0;

int assert(int expected, int actual)
{
    if (expected != actual) {
        printf("error: expected: %d actual: %d\n", expected, actual);
        exit(1);
    }
    return 0;
}

void set_count(int val)
{
    g_count = val;
}
