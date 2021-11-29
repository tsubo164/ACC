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
union bar { int i;};

int main()
{
    /*
    struct point p;
    return p.x;
    struct point p;
    struct point p = {19};
    union foo f = {42};
    union foo f, *fp = NULL;
    union foo g, *gp = NULL;
    */
    union foo *fp = NULL;
    int *ip = fp;

    union foo f;
    union bar b = f;

    return *ip;
    /*
    g = f;
    gp = fp;
    gp = ip;
    g = b;

    return f.i;
    */
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
