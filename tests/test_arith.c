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

int ptrcmp(int *x, int *y)
{
    return x == y;
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
    {
        /* modulo operator */
        int a = 12321;
        char b = 34;

        assert(13, a % b);

        a = -123534;
        b = 37;
        assert(-28, a % b);
    }
    {
        /* modulo operator */
        short a = 1219;
        unsigned char b = 233;

        assert(54, a % b);
    }
    {
        /* modulo operator */
        long a = 233334012321;
        unsigned short b = 2334;

        assert(1503, a % b);

        a = -123534233349;
        b = 337;
        assert(-124, a % b);
    }
    {
        /* modulo operator */
        int a = 15;
        int b = 4;

        a %= b;
        assert(3, a);

        a = 24145;
        b = 1233;
        a %= b;
        assert(718, a);
    }
    {
        /* implicit conversion from logical ops with pointer type */
        int *p = (void *) 0;
        int a = !p;

        assert(1, a);
        assert(1, !p && 1);
    }
    {
        /* convert equality of two pointers to int */
        int a = 1321;
        int *p = &a;
        int *q = &a;

        assert(1, ptrcmp(p, q));
        assert(1321, *p);
        assert(1321, *q);
    }
    {
        /* shfit left */
        int a = 1;
        int n = 3;

        enum {
            A = 1 << 0,
            B = 1 << 1,
            C = 1 << 2
        };

        a = a << 2;
        assert(4, a);

        a = a << 4;
        assert(64, a);

        a = 1 << n;
        assert(8, a);

        a <<= 4;
        assert(128, a);

        a <<= n;
        assert(1024, a);

        assert(1, A);
        assert(2, B);
        assert(4, C);
    }
    {
        /* shfit right */
        int i = -256;
        unsigned int u = 512;
        int n = 3;

        i = i >> 2;
        assert(-64, i);

        i = i >> 4;
        assert(-4, i);

        u = u >> n;
        assert(64, u);

        u >>= 2;
        assert(16, u);

        i = -512;
        i >>= n;
        assert(-64, i);
    }
    {
        /* bit or */
        enum {
            A = 1 << 0,
            B = 1 << 1,
            C = 1 << 2
        };
        int a = A;

        assert(3, A | B);
        assert(5, A | C);
        assert(6, B | C);
        assert(7, A | B | C);

        a |= C;
        assert(5, a);
    }
    {
        /* bit xor */
        enum {
            A = 1 << 0,
            B = 1 << 1,
            C = 1 << 2
        };

        int a = B | C;
        int b = A | B;

        assert(5, a ^ b);

        assert(6, a);
        a ^= b;
        assert(5, a);
    }
    {
        /* bit and */
        enum {
            A = 1 << 0,
            B = 1 << 1,
            C = 1 << 2
        };

        int a = B | C;
        int b = A | B;

        assert(2, a & b);

        assert(6, a);
        a &= b;
        assert(2, a);
    }
    {
        /* bit not */
        int a = -1;

        assert(0, ~a);

        a = 0;
        assert(-1, ~a);
    }

    return 0;
}
