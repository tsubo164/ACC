/*
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
*/

int main()
{
    char a = 10;
    short b = 20;
    int c = 30;

    a = a > 0 ? a++, b++, c++ : -1;

    ((void)0);

    return a + b + c;
}
