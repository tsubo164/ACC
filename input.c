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

int main()
{
    /*
    struct point p;
    return p.x;
    struct point p;
    struct point p = {19};
    union foo f = {42};
    */
    union bar b;
    union foo f, *fp = NULL;
    union foo g, *gp = NULL;
    int *ip;

    g = f;
    gp = fp;
    gp = ip;

    return f.i;
    /*

    f.i = 42;
    */

    /*
    printf("f.i: %d\n", f.i);

    f.s = "string";
    printf("f.s: %s\n", f.s);
    printf("sizeof(union foo): %ld\n", sizeof(union foo));
    return f.i;
    */

    return 13;
}
