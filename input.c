/*
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
*/

union foo {
    int i;
    long l;
    const char *s;
};

typedef union coord {
    const char *name;
    struct {
        long x, y, z;
    } p;
} Coord;

typedef union SC {
/*
typedef struct SC {
*/
    short a;
    char b;
} SC;

int main()
{
    return sizeof(SC);
}
