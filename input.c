/*
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
*/
#include <stdio.h>
/*
*/

struct point {
    int x, y, z;
};

union foo {
    int i;
    long l;
    const char *s;
    struct point p;
};

union bar;
/*
union bar { int i;};
*/

struct point func(struct point p)
{
    return 23;
}

int main()
{
    func(23);
    /*
    union bar b;
    union foo f;
    union foo f;
    union foo f;
    int i;
    int i;
    int i;

here:
here:
here:
    */

    return 12;
}
