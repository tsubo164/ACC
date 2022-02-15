/*
#include <stdio.h>
*/

int main()
{
    char c = 'A';

    if (!c)
        return 42;

    c = c;
    switch (c) {
    default:
        break;
    }

    while (c) {
        break;
    }

    return c;
}
