/*
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
*/

#define EOF (-1)

int main()
{
    int c = -1;

    switch (c) {
    case '\'':
        break;

    case 'a':
        break;

    case EOF:
        return 19;
        break;

    default:
        break;
    }

    return 42;
}

/*
void foo();
static int foo(void);

int main()
{
    int a = bar();

    return foo();
}

static int foo(void)
{
    return 42;
}
*/
