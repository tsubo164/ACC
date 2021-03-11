#include "test.h"

int main()
{
    {
        int sum = 0;
        int i;

        for (i = 1; i <= 10; i++) {
            sum = sum + i;
        }
        assert(55, sum);
    }
    {
        int i;

        for (i = 0; i < 10; ) {
            i = 2 * i + 1;
        }

        assert(15, i);
    }
    {
        int sum = 0;
        int i = 11;

        for (; i <= 20; ) {
            sum = sum + i;
            i++;
        }

        assert(155, sum);
    }
    {
        int i;

        for (i = 0; i < 10; i++)
            if (i == 6)
                break;
        assert(6, i);
    }
    {
        int i = 0;

        for (;;) {
            if (i / 3 == 4)
                if (i / 6 == 2)
                    break;
            i++;
        }
        assert(12, i);
    }
    {
        int sum = 0;
        int i = 0;

        for (i = 0; i < 20; i++) {
            if (i / 3 == 3) {
                sum += i;
                continue;
            }
        }
        assert(30, sum);
    }

    /*
    for (;;) {}
    */
    return 0;
}
