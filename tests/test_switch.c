#include "test.h"

enum {
    A, B, C, D
};

int foo(int a)
{
    /* consecutive case statement */
    switch (a) {
    case A:
    case B:
        return 19;
    case C:
    case D:
        return 23;
    default:
        return 29;
    }
}

int bar(int a, int b)
{
    /* consecutive case statement and nested switch */
    switch (a) {
    case A:
    case B:
        switch (b) {
        case A:
            return 119;
        case D:
            return 211;
        default:
            break;
        }
        return -1;

    case C:
    case D:
        return 23;
    default:
        return 29;
    }
}

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
    {
        /* switch with case value 0 */
        int a = 0;

        switch (a) {
        case 0:
            a = 10;
            break;
        case 1:
            a = 11;
            break;
        default:
            break;
        }

        assert(10, a);
    }
    {
        /* consecutive case statement */
        assert(19, foo(A));
        assert(19, foo(B));
        assert(23, foo(C));
        assert(23, foo(D));
        assert(29, foo(238));
    }
    {
        /* consecutive case statement and nested switch */
        assert(119, bar(A, A));
        assert(-1,  bar(A, B));
        assert(211, bar(B, D));
        assert(211, bar(A, D));
    }
    {
        /* switch without default case */
        int a = 3;

        switch (a) {
        case 0:
            a = 10;
            break;
        case 1:
            a = 11;
            break;
        }

        assert(3, a);
    }

    return 0;
}
