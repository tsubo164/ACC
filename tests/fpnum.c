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

    return 0;
}
