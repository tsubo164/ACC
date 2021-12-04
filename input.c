/*
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
*/
#include <stddef.h>

int main()
{
    char s[] = "abc";
    char *p = s + 2;

    ptrdiff_t len = p - s;

    return len;
}
