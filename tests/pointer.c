#include "test.h"
/* TODO implement standard include paths */
#include "../include/stddef.h"

int main()
{
    {
        int a;
        int *b;

        a = 40;
        b = &a;

        *b = *b + 2;

        assert(42, a);
    }
    {
        int a[4];
        int *p = a;

        a[0] = 11;
        a[1] = 22;
        a[2] = 33;
        a[3] = 44;

        assert(11, *p++);
        assert(22, *p);

        p++;
        assert(33, *p);
        assert(33, *p++);
        assert(44, *p);

        assert(44, *p--);
        assert(33, *p);
        assert(33, (*p)--);
        assert(32, *p);
    }
    {
        int a[4];
        int *p = a;

        a[0] = 11;
        a[1] = 22;
        a[2] = 33;
        a[3] = 44;

        assert(22, *++p);
        assert(22, *p);

        p++;
        assert(22, *--p);
        assert(33, *++p);
        assert(44, *++p);
        assert(43, --*p);
    }
    {
        char a[4];
        char *p = a;

        a[0] = 11;
        a[1] = 22;
        a[2] = 33;
        a[3] = 44;

        assert(11, *p++);
        assert(22, *p);

        p++;
        assert(33, *p);
        assert(33, *p++);
        assert(44, *p);

        assert(44, *p--);
        assert(33, *p);
        assert(33, (*p)--);
        assert(32, *p);
    }
    {
        char a[4];
        char *p = a;

        a[0] = 11;
        a[1] = 22;
        a[2] = 33;
        a[3] = 44;

        assert(22, *++p);
        assert(22, *p);

        p++;
        assert(22, *--p);
        assert(33, *++p);
        assert(44, *++p);
        assert(43, --*p);
    }
    {
        int i = 13;
        int *p;
        int **pp;
        int ***ppp;

        p = &i;
        pp = &p;
        ppp = &pp;

        *p = 13;
        assert(13, *p);

        **pp = *p + 11;
        assert(24, **pp);

        ***ppp = **pp + 18;
        assert(42, **pp);
    }
    {
        /* multiple init declarators */
        int *p, i = 0;

        p = &i;
        *p = 42;

        assert(42, i);
        assert(42, *p);
        assert(4, sizeof i);
        assert(8, sizeof p);
    }
    {
        /* multiple init declarators */
        int a[3], i = 42, *p = 0;

        p = a;
        a[0] = 11;
        a[1] = 21;
        a[2] = 31;

        assert(11, a[0]);
        assert(21, a[1]);
        assert(31, a[2]);
        assert(11, *p);
        assert(12, sizeof a);
        assert(4, sizeof i);
        assert(8, sizeof p);
    }
    {
        /* pointer subtraction to integer type */
        char s[] = "abcdef";
        char *p = s + 4;
        ptrdiff_t d = p - s;

        assert(4, d);
    }

    return 0;
}
