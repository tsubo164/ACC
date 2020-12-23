int assert(int expected, int actual);

enum color {
    R,
    G,
    B
};

int main()
{
    assert(0, R);
    assert(1, G);
    assert(2, B);

    {
        enum color {
            R = 10,
            G,
            B
        };
        assert(10, R);
        assert(11, G);
        assert(12, B);
    }
    {
        enum color {
            R = 10,
            G,
            B = R + G
        };
        assert(10, R);
        assert(11, G);
        assert(21, B);
    }
    {
        enum axis {
            X = -1,
            Y = 9,
            Z = 2 * X - Y / 3
        };
        assert(-1, X);
        assert( 9, Y);
        assert(-5, Z);
    }
    {
        enum axis {
            X,
            Y,
            Z = X * Y
        };
        assert(0, X);
        assert(1, Y);
        assert(0, Z);
    }
    {
        int a = R;
        int b = G;
        int c = B;

        assert(8, a + 2 * b + 3 * c);
    }

    return 0;
}
