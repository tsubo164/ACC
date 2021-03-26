#include <stdio.h>

int a = 42;

int main()
{
    int *p = NULL;

    void *v = p;

    v = &a;

    return *((int *)v);
}
