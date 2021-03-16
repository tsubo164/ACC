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
    char c = 'c';
    int i = -123;
    long l;

    l = i;
    printf("l => %ld\n", l);

    i = c;
    printf("c => %d\n", c);

    i = -111;
    bar(i);

    foo(l);

    return 0;
}
