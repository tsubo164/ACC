/*
#include <stdio.h>
*/
#include <stdio.h>

struct bitfield3 {
    unsigned int a : 10;
    signed int : 0;
    unsigned int b : 10;
    int c;
} bf3 = {520, 1023, -23242};

int main()
{
    printf("bf3.a %d\n", bf3.a);
    return 3;
}
