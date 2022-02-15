/*
#include <stdio.h>
*/

int main()
{
    enum etag {A, B, C } e;
    char c = 'A';

    if (c)
        return 42;

    c = c;
    switch (e) {
    default:
        break;
    }

    while (c) {
        break;
    }

    while (A) {
        break;
    }
    for (; c; ) {
        break;
    }

    if (!c)
        return 42;

    if (c == c)
        return 42;

    return c;
}
