int foo()
{
    int i;
    i = 0;

    while (i < 10) {
        i = 2 * i + 1;
    }

    return i; /* 15 */
}

int num()
{
    int n;
    n = 7;
    return n;
}

int add(int x, int y)
{
    return x + y;
}

int main()
{
    int a;
    int b;
    int c;
    int d;

    a = add(21, 7) +  2 * num(); /* 42 */

    b = 42 / num() < 7; /* 1 */

    c = foo() > 14; /* 1 */

    if (a + b + c == 44)
        return 0;
    else
        return 1;
}
