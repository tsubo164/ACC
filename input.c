/*
*/
#include <stdio.h>

int main()
{
    struct bitfield2 {
        signed int a : 10;
        signed int b : 10;
    } bf2 = {73, 31};

    printf("bf2.a => %d\n", bf2.a);
    printf("bf2.b => %d\n", bf2.b);

    return sizeof bf2;
}
