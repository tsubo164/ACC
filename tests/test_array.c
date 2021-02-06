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
    {
        int a[3][2];
        int b[4][3][2];

        a[2][1] = 13;
        b[3][2][0] = 29;

        assert(13, a[2][1]);
        assert(29, b[3][2][0]);
        assert(42, a[2][1] + b[3][2][0]);
    }
    {
        struct vec {
            int x, y;
        } a[3][2];

        a[2][1].x = 13;
        a[2][1].y = 31;

        assert(13, a[2][1].x);
        assert(31, a[2][1].y);
    }

    return 0;
}
