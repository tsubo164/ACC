/*
#include <stdio.h>
*/

/*
void assert(int expected, int actual);
int g_count = 12;
*/
int num()
{
    return 23;
}


int main()
{
    /*
    assert(1, 1);
    */
    int (*fp2)();

    fp2 = num;

    return 0;
    /*
    return num();
    return g_count;
    */
}
