#include "input.h"

void foo(int i)
{
    printf("%d\n", i);
}

void bar(long l)
{
    printf("%ld\n", l);
}

int main()
{
    long l = 123322243204932;

    //int i = l;

    //printf("%ld\n", l);
    //foo(l);
    bar(l);

    return l;
}
