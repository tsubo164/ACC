/*
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
*/

int main()
{
    char a = 10;
    char b = 20;

    /* need () around the third expression */
    (a > 10) ? a = 42 : (a = 19);

    return a;
}
