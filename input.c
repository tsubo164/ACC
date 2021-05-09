/*
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
*/

typedef struct point {
    int x, y, z, w;
} Point;

typedef struct vector {
    int x, y, z;
} Vector;

int foo()
{
    return 42;
}

int a = foo();
/*
[-] init struct by struct (only for local)
[ ] check type for assign
[x] check different struct assignment
[ ] copy struct
*/
int main()
{
    Point p = {11, 22, 33, 44};//, q;
    Point r = p;
    //Vector v = p;
    int i = 17;

    /*
    q = p;
    v = p;
    */

    //return 17;
    //return q.z;
    return r.w;
}
