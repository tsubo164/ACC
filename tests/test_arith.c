#include "test.h"

int num()
{
    int n;
    n = 1;
    return n;
}

int add(int x, int y)
{
    return x + y;
}

int main()
{
    {
        int ret;
        ret = add(40, 2) + 2 * num();

        assert(44, ret);
    }
    {
        assert(23, 21 + 2 * 3 - 8/2);
    }
    {
        int a = -1;

        a += 1;

        assert(0, a++);
        assert(1, a);
        assert(42, a += 41);
    }
    {
        int a = -1;

        a -= 1;

        assert(-2, a++);
        assert(-1, a);
        assert(-42, a -= 41);
    }
    {
        int a = 12;

        a *= 4;

        assert(48, a++);
        assert(49, a);
        assert(98, a *= 2);
    }
    {
        int a = 12;

        a /= 4;

        assert(3, a);
        assert(1, a /= 3);
    }
    {
        int a = 7;
        int b = 2;
        assert(1, a && b);
        a = 0;
        b = 6;
        assert(0, a && b);
        a = 9;
        b = 0;
        assert(0, a && b);
        a = 0;
        b = 0;
        assert(0, a && b);

        a = 7;
        b = 2;
        assert(1, a || b);
        a = 0;
        b = 6;
        assert(1, a || b);
        a = 8;
        b = 0;
        assert(1, a || b);
        a = 0;
        b = 0;
        assert(0, a || b);
    }
    {
        int a = 0;
        int b = 7;
        int c;

        c = a && b++;
        assert(0, c);
        assert(7, b);

        a = 1;
        c = (a && b++);
        assert(1, c);
        assert(8, b);
    }
    {
        int a = 0;
        int b = -12;
        int c;

        c = a || b++;
        assert(1, c);
        assert(-11, b);

        a = 1;
        c = (a || b++);
        assert(1, c);
        assert(-11, b);
    }
    {
        int a = 0;
        int b = -12;
        int c;

        c = a || b++;
        assert(0, !c);

        a = 1;
        c = (a || b++);
        assert(0, !c);
    }
    {
        int a = 2;

        assert(111, a < 10 ? 111 : a > 50 ? 1111 : 19);

        a = 52;
        assert(1111, a < 10 ? 111 : a > 50 ? 1111 : 19);

        a = 23;
        assert(19, a < 10 ? 111 : a > 50 ? 1111 : 19);

        assert(1, a < 10 ? 0 : 1);

        assert(1, 0 ? 0 : 1);
        assert(0, 3 ? 0 : 1);
    }
    {
        /* usual arithmetic conversion */
        char c;
        short s;
        int i;
        long l;

        c = -1;
        assert(-1, c);

        c = -2;
        s = c;
        assert(-2, s);

        c = -4;
        i = c;
        assert(-4, i);

        c = -8;
        l = c;
        assert(-8, l);
    }

    return 0;
}
