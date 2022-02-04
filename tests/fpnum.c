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

    return 0;
}
