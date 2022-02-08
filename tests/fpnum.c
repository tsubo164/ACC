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
        float f = 2.71828;

        assertf(2.71828, f);
    }

    return 0;
}
