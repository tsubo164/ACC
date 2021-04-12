/*
#include <string.h>
#include <stdlib.h>
*/

#include <stdio.h>

enum {
    A,
    B,
    C
};

const char *foo(int kind)
{
    switch (kind) {
    case A: return "A"; break;
    case B: return "B"; break;
    case C: return "C"; break;
    default: "NULL"; break;
    }
    return "NULL";
}

int main()
{
    int kind = 0;

    const char *s = foo(kind);
    printf("    kind => %s\n", s);

    /*
    char *s = "abc";
    int i = 42;
    printf("%s-%d-%d\n", s, i, i / 2);
    int a = 0;

    switch (a) {
    case 0:
        break;
    case 1:
        break;
    default:
        break;
    }
    */

    return 21;
}
