int assert(int expected, int actual);

/* array index access */
int main()
{
    {
        int a[4];

        a[0] = 3;
        a[1] = 7;

        a[2] = a[0] + a[1];
        a[3] = a[2] * 4 + 2;

        assert(10, a[2]);
        assert(42, a[3]);
        assert(42, *(a+3));
        assert(16, sizeof a);
        assert(4, sizeof a[2]);
    }
    {
        enum length {
            ARRAY_LENGTH = 3
        };

        /* array length with constant expression */
        int a[3 + 1];
        int b[ARRAY_LENGTH];

        a[3] = 42;
        b[2] = 19;

        assert(42, a[3]);
        assert(19, b[2]);
        assert(23, a[3] - b[2]);
    }

    return 0;
}
