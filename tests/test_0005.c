/* global variables and pointer */

int add(int x, int y)
{
    return x + y;
}

int x;
int z;

int main()
{
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
