int assert(int expected, int actual);

/* no specified length array at the beginning of function */
int test_single_array()
{
    int a[] = {345, 41, 399};

    assert(12, sizeof a);
    assert(345, a[0]);
    assert(41,  a[1]);
    assert(399, a[2]);
}

int main()
{
    {
        /* array index access */
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
    {
        /* array initializer with multi-dimension */
        int a[2][2] = {{11, 22}, {44, 55}};

        assert(16, sizeof a);
        assert(11, a[0][0]);
        assert(22, a[0][1]);
        assert(44, a[1][0]);
        assert(55, a[1][1]);
    }
    {
        /* multi-dimensional array initializer with less than specified size */
        int a[3][2] = {{111, 222}, {444, 555}};

        assert(24, sizeof a);
        assert(111, a[0][0]);
        assert(222, a[0][1]);
        assert(444, a[1][0]);
        assert(555, a[1][1]);
        assert(  0, a[2][0]);
        assert(  0, a[2][1]);
    }
    {
        test_single_array();
    }

    return 0;
}
