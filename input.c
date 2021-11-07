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
int main()
{
    int a = 42;
    //`a = 3;
    ;

    while (0)
        ;
    return a;
}
