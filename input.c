/*
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
*/

#define EOF (-1)

int main()
{
    const char *s = "this is EOF\n";
    int i = EOF;

    switch (i) {
    case '"':
        return 29;

    case '\'':
        return 29;

    case EOF:
        return 13;

    default:
        break;
    }

    foo();

    return 42;
}
