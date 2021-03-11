#include "test.h"

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
        /* enum and sizeof */
        enum color {
            R = 10,
            G,
            B
        };
        assert(10, R);
        assert(11, G);
        assert(12, B);

        assert(4, sizeof R);
        assert(4, sizeof (enum color));
    }
    {
        /* enum with constant expression */
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
        /* enum with constant expression */
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
        /* enum with constant expression */
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
        /* lookup global enum */
        int a = R;
        int b = G;
        int c = B;

        assert(8, a + 2 * b + 3 * c);
    }
    {
        /* enum with no tag */
        enum {
            red = 101, green, blue
        } col;

        col = green;

        assert(102, col);
    }
    {
        /* variable with enum specifier */
        enum color {
            red = 101, green, blue
        };

        enum color c = blue;

        assert(103, c);
    }

    return 0;
}
