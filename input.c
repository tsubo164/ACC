/*
#include <stdio.h>
*/

int main()
{
    enum etag {A, B, C = 1232432144412 } e;
    char c = 'A';

    if (!c)
        return 42;

    c = c;
    switch (e) {
    default:
        break;
    }

    while (c) {
        break;
    }

    return c;
}
