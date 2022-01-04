/*
#include <stdio.h>
*/

struct point {
    int x, y;
    int bit : 3 + 2;
};

enum foo {
    A = 19 + 8, B, C
};

struct point p;

int add(int x, int y)
{
    return x + y;
}

int main()
{
    int i = 40, j = 2;

    return i + j;
}
