/*
#include <stdio.h>
*/

#include <stdio.h>

void foo(float f)
{
    printf("%g\n", f);
}
/*
*/
int add(int a, int b)
{
    return a - b;
}

int main() {
    float f;

    f = 19;

    foo(f);
    /*
    printf("%g\n", f);
    */

    return add(33, 22);
}
