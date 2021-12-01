/*
#include <stdlib.h>
#include <string.h>
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

/*
union foo f = {42};
*/
int main()
{
    /*
    struct point p = {19, 29, 39};
    */
    union foo f = {42};
    union foo g = f;

    /*
    f.p = p;

    g = f;
    */

    return g.i;
    /*
    return f.p.z;
    */
}
