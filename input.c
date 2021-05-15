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
[x] copy struct
*/
int main()
{
    Point p = {11, 22, 33, 44};//, q;
    Point q;

    q = p;

    return q.w;
}
