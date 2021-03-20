#include "input.h"

int main()
{
    char c;
    short s;
    int i;
    long l;

    c = -1;
    printf("------ %d\n", c);

    c = -2;
    s = c;
    printf("------ %d\n", c);

    c = -4;
    i = c;
    printf("------ %d\n", i);

    c = -8;
    l = /*(long)*/ c;
    printf("------ %ld\n", l);

    return 0;
}
