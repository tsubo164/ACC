/* global variables and pointer */

int x;

int add(int x, int y)
{
    return x + y;
}

int z;

int main()
{
    int x;

    int a;
    int *p;

    p = &x;

    *p = 3; /* x = 3 */

    a = add(39, x); /* a = 42 */

    z = a; /* z = 42 */

    if (z + *p == 45)
        return 0;
    else
        return 1;
}
