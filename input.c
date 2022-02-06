/*
#include <stdio.h>
*/

#include <stdio.h>

void foo(float f)
{
    printf("%f\n", f);
}

/*
void bar(int i)
{
    printf("%d\n", i);
}
*/
struct bitfield1 {
    signed int a : 13;
    signed int b : 17;
} bf1 = {-73, 991};


int main()
{
    float f = 3.14;
    float g = 0.5;
    float h = 0.14;
    //int i = 1;

    //printf("%f\n", f);
    foo(f + g);
    foo(f - g - h);
    printf("%d\n", bf1.a);

    return 42;
}
