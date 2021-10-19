/*
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
*/

#define EOF (-1)

int main()
{
    const char *str = "this is /* string \"literal\"\n";
    int i = EOF;

    switch (i) {
    case EOF:
        return 13;
    default:
        break;
    }

    return 42;
}
