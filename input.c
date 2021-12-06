/*
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
*/

int cnt = 42;

char *p = "world!\n";

int main()
{
    static int depth = 0;
    int a = cnt;

    char *s = "hello\n";

    return a + s[0] + depth;
}
