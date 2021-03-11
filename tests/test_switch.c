#include "test.h"

int main()
{
    {
        int a = 11;
        int b = 100;

        switch (a) {
        case 1:
            return a;
        case 2:
            break;
        case 11: /* jump to here */
            switch (a + b) {
            case 110:
                a *= 2;
                break;
            case 111: /* jump to here */
                a *= 3;
                break;
            case 1:
                a *= 4;
                break;
            case 2:
                a *= 5;
                break;
            case 11:
                a -= 5;
                break;
            }
            b += -50;
            break;
        case 13:
            return a;

        default:
            break;
        }

        assert(33, a);
        assert(50, b);
    }
    {
        int a = 1;
        int b = 100;

        switch (a) {
        case 1:
            a += 10;
        case 2:
            a += 13;
        case 11: /* fall through to here */
            a += 37;
            break;
        case 13:
            a += 97;
            break;

        default:
            break;
        }

        assert(61, a);
        assert(100, b);
    }
    {
        /* default case */
        int a = 42;

        switch (3) {
        case 19:
            a = 3;
            break;
        case 23:
            a = 4;
            break;
        default:
            break;
        }

        assert(42, a);
    }

    return 0;
}
