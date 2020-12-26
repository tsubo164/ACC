int assert(int expected, int actual);

int foo()
{
    int i = 0;

    while (i < 10) {
        i = 2 * i + 1;
    }

    assert(15, i);
    return i;
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
    {
        int a;
        int b;
        int c;

        a = add(21, 7) +  2 * num();
        assert(42, a);

        b = 42 / num() < 7;
        assert(1, b);

        c = foo() > 14;
        assert(1, c);
        assert(44, a + b + c);
    }
    {
        int sum = 0;
        int i = 0;

        while (i < 10) {
            i++;
            sum = sum + i;
        }

        assert(55, sum);
    }
    {
        int sum = 0;
        int i = 0;

        do {
            i++;
            sum = sum + i;
        } while (i < 10);

        assert(55, sum);
    }
    {
        int i = 0;

        do ++i; while (i < 10);

        assert(10, i);
    }

    return 0;
}
