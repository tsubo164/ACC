/*
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
*/
/*
#include <stdio.h>
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

int main()
{
    union foo f = {42};

    /*
    printf("f.i: %d\n", f.i);

    f.s = "string";
    printf("f.s: %s\n", f.s);
    printf("sizeof(union foo): %ld\n", sizeof(union foo));
    */

    return f.i;
}
