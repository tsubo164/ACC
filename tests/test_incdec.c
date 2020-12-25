int assert(int expected, int actual);

int main()
{
    {
        int a = -1;

        assert(0, ++a);
        assert(-1, --a);
        assert(-2, --a);

        a = 221134;

        assert(221135, ++a);
        assert(221134, --a);
        assert(221133, --a);
    }

    {
        int a = -111;
        int b = 130;

        assert(-110, ++a);
        assert(-109, ++a);
        assert(-110, --a);

        b = b + a;

        assert(20, b);
        assert(21, ++b);
        assert(20, --b);
    }
    return 0;
}
