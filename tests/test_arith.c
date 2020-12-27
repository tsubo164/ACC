int assert(int expected, int actual);

int num()
{
    int n;
    n = 1;
    return n;
}

int add(int x, int y)
{
    return x + y;
}

int main()
{
    {
        int ret;
        ret = add(40, 2) + 2 * num();

        assert(44, ret);
    }
    {
        assert(23, 21 + 2 * 3 - 8/2);
    }
    {
        int a = -1;

        a += 1;

        assert(0, a++);
        assert(1, a);
        assert(42, a += 41);
    }
    {
        int a = -1;

        a -= 1;

        assert(-2, a++);
        assert(-1, a);
        assert(-42, a -= 41);
    }
    {
        int a = 12;

        a *= 4;

        assert(48, a++);
        assert(49, a);
        assert(98, a *= 2);
    }
    {
        int a = 12;

        a /= 4;

        assert(3, a);
        assert(1, a /= 3);
    }

    return 0;
}
