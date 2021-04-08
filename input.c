/*
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
*/

char s[3] = "abc";

char *str = "abc";

struct foo {
    int i;
    char s[8];
} f = {42, "FOO"};

int main()
{
    /*
    int a = 19;
    short s[] = {11, 22};
    */

    return f.i;
}
