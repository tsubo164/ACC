/*
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
*/

/* testing hideset for one macro apearing twice */
/*
#define SEVEN (7)
#define OP(op) (SEVEN op SEVEN)
OP(+)

#define A B
#define B C
#define C A
A
*/

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

int foo(int a, int b, int c)
{
    return a + b + c;
}

int bar(int a)
{
    return 2 * a;
}

int main()
{
    int a;

    a = foo(11, 22, bar(33));

    return a;
}
