/*
#include <stdio.h>
*/

int array_to_pointer(int a[1])
{
    return *a;
}

int array_to_pointer2(char a[1][1])
{
    return **a;
}

int main()
{
    return 0;
}
