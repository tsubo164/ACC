/* global variables and pointer */

/*
int x;

int add(int x, int y)
{
    return x + y;
}

int z;
*/

int main()
{
    int x[2];
    int a;
    /*
    */

    /*
    x[0] = 7;
    a = 7;
    */
    x[0] = 2;
    *(x + 1) = -7;

    /*
    return 0;
    */
    x[0] = x[1] - x[0];

    a = 51;

    if (a + x[0] == 42)
        return 0;
    else
        return 1;
}
