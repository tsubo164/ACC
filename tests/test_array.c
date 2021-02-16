int assert(int expected, int actual);

/* array index access */
int main()
{
    {
        /* basic array */
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
        /* multi-dimensional array */
        int a[3][2];
        int b[4][3][2];

        a[2][1] = 13;
        b[3][2][0] = 29;

        assert(13, a[2][1]);
        assert(29, b[3][2][0]);
        assert(42, a[2][1] + b[3][2][0]);
    }
    {
        /* array of struct */
        struct vec {
            int x, y;
        } a[3][2];

        a[2][1].x = 13;
        a[2][1].y = 31;

        assert(13, a[2][1].x);
        assert(31, a[2][1].y);
    }
    {
        /* array initializer */
        int a[3] = {11, 99, 31};

        assert(11, a[0]);
        assert(99, a[1]);
        assert(31, a[2]);
    }
    {
        /* array initializer with less than specified size */
        int a[8] = {5, 37, 19};

        assert(5, a[0]);
        assert(37, a[1]);
        assert(19, a[2]);
        assert(0, a[3]);
        assert(0, a[4]);
        assert(0, a[5]);
        assert(0, a[6]);
        assert(0, a[7]);
    }
    {
        /* array initializer with unknown size */
        int a[] = {15, 37, 19, 23};

        assert(15, a[0]);
        assert(37, a[1]);
        assert(19, a[2]);
        assert(23, a[3]);

        assert(16,  sizeof a);
    }

    return 0;
}
