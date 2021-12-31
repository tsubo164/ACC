/*
#include <stdio.h>
*/

int twice(int i)
{
    return 2 * i;
}

int main()
{
    int (*fp)(int) = twice;

    return fp(21);
}
