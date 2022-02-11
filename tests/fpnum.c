#include "test.h"

int main()
{
    {
        /* fpnum to int */
        float f;
        double d = 4.14;

        f = 3.14;

        assert(3, f);
        assert(4, d);
    }
    {
        /* fpnum initialization */
        float f = 2.71828;
        double d = 2394.234803;

        assertf(2.71828, f);
        assertd(2394.234803, d);
    }
    {
        /* arithmetic operations */
        float f = 1.234;
        double d = 0.23;

        assertf(1.464, f + d);
        assertf(1.004, f - d);
        assertf(0.28382, f * d);
        assertf(5.3652173913043475, f / d);
    }
    {
        /* relational operations */
        float f = 2.34;
        double d = 3.84;

        assert(1, f > 2.3);
        assert(1, f < 2.3 * 2);
        assert(0, f < 2.3 - 0.3);

        assert(1, d <= 3.84000);
        assert(1, d >= 3.84000);

        assert(0, d <= 3.84000 * 0.5);
    }
    {
        /* equality operations */
        double f = 9.87;

        assert(1, f == 9.87);
        assert(0, f == 9.78);
        assert(0, f != 9.87);
        assert(1, f != 9.78);
    }

    return 0;
}
