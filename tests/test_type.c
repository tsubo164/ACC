#include "test.h"

void foo()
{
    int a = 1;

    if (a > 0)
        return;

    return;
}

struct point {
    int x, y, z;
};

enum color {
    R, G, B
};

typedef int id_t;

int main()
{
    {
        /* char data type */
        char x[2];
        int a;

        x[0] = 2;
        assert(2, x[0]);

        *(x + 1) = -7;
        assert(-7, x[1]);

        x[0] = x[1] - x[0];
        assert(-9, x[0]);

        a = 51;
        assert(42, a + x[0]);
    }
    {
        /* char literal */
        char c = 'a';

        assert(97, c);

        c = '\0';
        assert(0, c);

        c = '\n';
        assert(10, c);

        c = '\\';
        assert(92, c);

        c = '\'';
        assert(39, c);

        c = 'X';
        assert(88, c);
    }
    {
        /*
        void i = 0;
        */
    }
    {
        /* sizeof struct */
        struct point pt;
        int a = 42;
        int *p = &a;
        char c = 92;

        assert(4, sizeof a);

        assert(4, sizeof(a + 2));

        assert(8, sizeof p);

        assert(4, sizeof *p);

        assert(1, sizeof c);

        assert(12, sizeof pt);

        assert(4, sizeof pt.x);

        assert(12, sizeof (struct point ));

        assert(4, sizeof R);

        assert(4, sizeof (enum color));
    }
    {
        /* sizeof typdedef */
        typedef char token_t;

        id_t id = 123;
        token_t tok = 97;

        assert(123, id);
        assert(4, sizeof id);
        assert(4, sizeof(id_t));

        assert(97, tok);
        assert(1, sizeof tok);
        assert(1, sizeof(token_t));
    }
    {
        /* const static */
        const static int i = 42;
        assert(42, i);
    }
    {
        /* short int */
        short s = 4096;
        char c;

        assert(2, sizeof s);
        assert(2, sizeof(short));
        assert(4096, s);

        c = s;
        assert(0, c);
    }
    {
        /* long int */
        long l = 12321032434;

        assert(8, sizeof l);
        assert(8, sizeof(long));
        assertl(12321032434, l);
    }
    {
        /* signed extensions */
        char c = -19;
        int i = -123;
        long l;

        l = i;
        assert(-123, l);

        i = c;
        assert(-19, i);
    }
    {
        /* unsigned char */
        unsigned char uc = 250;
        signed char c = 250;

        assert(1, sizeof(uc));
        assert(250, uc);

        assert(1, sizeof(c));
        assert(-6, c);
    }
    {
        /* unsigned int */
        unsigned int ui = 4294967295;
        signed int i = 4294967295;

        assert(4, sizeof(ui));
        assert(4, sizeof(unsigned int));
        assert(-1, ui);

        assert(4, sizeof(i));
        assert(-1, i);
    }
    {
        /* cast */
        int i = 1030;
        assert(6, (char) i);

        i = 1223034;
        assert(-22150, (short) i);
    }
    {
        /* declarators */
        int a;
        short *b, c[3];
        int (*d)[3];
        long *e[3];
        char (f);
        long (**g[3]);
        long (*(*h)[3]);

        a = 1;
        b = 0;
        c[0] = 0;
        d = 0;
        e[0] = 0;
        f = 0;
        g[0] = 0;
        h = 0;

        assert(4, sizeof a);
        assert(8, sizeof b);
        assert(6, sizeof c);
        assert(8, sizeof d);
        assert(24, sizeof e);
        assert(1, sizeof f);
        assert(24, sizeof g);
        assert(8, sizeof h);
    }
    {
        /* typedef of array */
        typedef int Vec3[3];

        Vec3 v0 = {82, 32, 29};

        assert(12, sizeof(Vec3));
        assert(12, sizeof v0);
        assert(82, v0[0]);
        assert(32, v0[1]);
        assert(29, v0[2]);
    }
    {
        /* typedef of pointer and more */
        typedef struct Point {
            int x, y;
        } Point;

        typedef int Array[3];
        typedef int Integer;
        typedef Integer ID;
        typedef int *Ref;
        Integer i = 32;
        Array a = {11, 22, 33};
        Point p = {99, 88};
        Ref r = &i;

        assert(12, sizeof(Array));
        assert(4,  sizeof(Integer));
        assert(4,  sizeof(ID));
        assert(8,  sizeof(Ref));

        assert(33, a[2]);
        assert(32, i);
        assert(88, p.y);
        assert(32, *r);
    }

    return 0;
}
