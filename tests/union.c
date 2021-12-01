#include "test.h"

struct point {
    int x, y, z;
};

union var {
    int i;
    long l;
    const char *s;
    struct point p;
};

int add(int x, int y)
{
    return x + y;
}

void add_var(union var *out, union var *p, union var *q)
{
    out->i = p->i + q->i;
}

int geti(union var *p)
{
    return p->i;
}

long getl(union var *p)
{
    return p->l;
}

int get_i3(union var v)
{
    return v.i;
}

long get_l3(union var v)
{
    return v.l;
}

const char *get_s3(union var v)
{
    return v.s;
}

union u8 {
    int i;
    long l;
    const char *s;
};

int geti_(union u8 p)
{
    return p.i;
}

long getl_(union u8 p)
{
    return p.l;
}

typedef union coord {
    const char *name;
    struct {
        long x, y, z;
    } p;
} Coord;

long coord_x(Coord c)
{
    return c.p.x;
}

long coord_y(Coord c)
{
    return c.p.y;
}

long coord_z(Coord c)
{
    return c.p.z;
}

int main()
{
    {
        /* union and pointer */
        union var v;
        union var *vp;

        v.i = 32;
        assert(32, v.i);

        vp = &v;
        vp->i = -123;
        assert(-123, vp->i);

        assert(-123, v.i);

        v.l = -22130193123;
        assertl(-22130193123, v.l);

        v.s = "Hello";
        assert('H', v.s[0]);
        assert('e', v.s[1]);
        assert('l', v.s[2]);
        assert('l', v.s[3]);
        assert('o', v.s[4]);
        assert('\0', v.s[5]);
    }
    {
        /* pointer access of union */
        union var v;
        union var *vp;

        vp = &v;

        vp->p.x = 9;
        assert(9, v.p.x);

        vp->p.y = 3;
        assert(3, v.p.y);

        assert(12, vp->p.x + vp->p.y);

        assert(8,  sizeof vp);
        assert(16, sizeof v);
        assert(16, sizeof *vp);
        assert(4,  sizeof vp->p.x);
        assert(16, sizeof (union var));
    }
    {
        /* array of union */
        union foo {
            int i, j, k;
        } f;

        union foo ff;
        union foo fa[3];

        f.i = 121;
        ff.j = 111;

        assert(121, f.i);
        assert(111, ff.j);
        assert(10, f.i - ff.j);
        assert(4, sizeof(union foo));

        /* j and i are sharing data */
        f.j = 99231;
        assert(99231, f.i);

        fa[1].j = 142;
        fa[2].k = 42;
        assert(142, fa[1].j);
        assert(42, fa[2].k);
    }
    {
        /* typedef'ed struct */
        typedef union var variant;
        variant v0, v1;
        variant *vp = &v1;

        v0.p.x = 123;
        v1.p.y = 23;

        assert(123, v0.p.x);
        assert(23, v1.p.y);

        vp->p.x = 31;
        vp->p.z = 91;

        assert(31, v1.p.x);
        assert(91, v1.p.z);
    }
    {
        /* nested union */
        typedef union klass {
            int x, y;
            char *s;
        } klass;

        union line {
            klass k0, k1;
        };

        struct line l;

        l.k0.x = 19;
        l.k1.y = 23;

        assert(23, l.k0.x);
        assert(23, l.k1.y);
        assert(46, l.k0.x + l.k1.y);
    }
    {
        /* union with no tag */
        union {
            int x, y;
            const char *s;
        } no_tag;

        no_tag.x = 111;
        no_tag.y = 19;

        assert(19, no_tag.x);
        assert(19, no_tag.y);
        assert(38, no_tag.x + no_tag.y);
    }
    {
        /* union initializer */
        union foo {
            int i;
            char *s;
        } f = {11};

        assert(11, f.i);

        f.s = "Hello";
        assert('H', f.s[0]);
        assert('e', f.s[1]);
        assert('l', f.s[2]);
        assert('l', f.s[3]);
        assert('o', f.s[4]);
        assert('\0', f.s[5]);
    }
    {
        /* union initializer with expressions */
        enum color {
            R = 11,
            G,
            B
        };
        union point {
            int x, y, z;
        } pt = {R + G + B}, pt2 = {add(4, 5)};

        assert(36, pt.x);
        assert(9, pt2.x);
    }
    {
        /* size of union with internal union members */
        union foo {
            int a;
            union bar {
                int b;
            } b;
        };

        assert(4, sizeof (union foo));
    }
    {
        /* union with unsigned int members */
        union foo {
            const unsigned int i, j;
        } f = {29};

        assert(4, sizeof (union foo));
        assert(29, f.i);
        assert(29, f.j);
    }
    {
        /* union pointer for functions */
        union var p = {42};
        union var q = {13};
        union var result;

        /* function calls in function arguments */
        assert(42, geti(&p));
        assert(42, p.i);

        q.l = 49;
        assertl(49, getl(&q));

        q.i = 13;
        add_var(&result, &p, &q);

        assert(55, geti(&result));

        result.l = 120;
        assertl(120, getl(&result));
    }
    {
        /* initialize union object with another union object */
        typedef union var Variant;

        Variant p = {79};
        Variant q = p;

        assert(79, p.i);
        assert(79, q.i);
    }
    {
        /* assign union object */
        typedef union var Variant;

        Variant p = {111};
        Variant q;

        q = p;

        assert(111, p.i);
        assert(111, q.i);
    }
    {
        /* 8 byte union for passing by value */
        union u8 p = {14};

        assert(14, geti_(p));
        assert(14, p.i);

        p.l = 17;
        assertl(17, getl_(p));
        assertl(17, p.l);
    }
    {
        /* 16 byte struct for passing by value */
        typedef union var var;
        var v = {41};

        assert(41, get_i3(v));
        assert(41, v.i);

        v.l = 71;
        assertl(71, get_l3(v));
        assertl(71, v.l);

        v.s = "\n";
        assert(10, get_s3(v)[0]);
    }
    {
        /* large struct for passing by value */
        Coord c; 

        assert(24, sizeof(c));

        c.p.x = 111;
        c.p.y = 222;
        c.p.z = 199;

        assert(111, coord_x(c));
        assert(111, c.p.x);
        assert(222, coord_y(c));
        assert(222, c.p.y);
        assert(199, coord_z(c));
        assert(199, c.p.z);
    }
#ifdef MORETEST
    {
        /* 8 byte struct returned by value */
        struct point p = get_point();

        assert(71, p.x);
        assert(92, p.y);
    }
    {
        /* 16 byte struct returned by value */
        vec v = get_vec();

        assert(1301, v.x);
        assert(223922, v.y);
        assert(-3973, v.z);
    }
    {
        /* large struct returned by value */
        Coord c = get_coord();

        assert(72340, c.x);
        assert(-1230889, c.y);
        assert(91355, c.z);
    }
    {
        /* copying struct through pointer dereference */
        vec v = {911, 822, 733};
        vec w;

        copy_vec(&w, &v);
        assert(911, w.x);
        assert(822, w.y);
        assert(733, w.z);
    }
    {
        /* testing no-constness of member that comes after const member */
        struct foo {
            const struct point *p;
            int i;
        } f;

        f.i = 123;
        assert(123, f.i);
    }
    {
        /* struct size and alignment */
        struct point {
            int x, y, z;
        };
        typedef struct C {
            char a;
        } C;
        typedef struct S {
            short a;
        } S;
        typedef struct SC {
            short a;
            char b;
        } SC;
        typedef struct I {
            int a;
        } I;
        typedef struct IC {
            int i;
            char a;
        } IC;
        typedef struct ICI {
            int i;
            char a;
            int j;
        } ICI;
        typedef struct ICS {
            int i;
            char a;
            short j;
        } ICS;
        typedef struct PC {
            void *p;
            char a;
        } PC;

        C c2[2];
        ICI ici2[2];

        assert(1, sizeof(C));
        assert(2 , sizeof(S));
        assert(4 , sizeof(SC));
        assert(4 , sizeof(I));
        assert(8 , sizeof(IC));
        assert(12, sizeof(ICI));
        assert(8 , sizeof(ICS));
        assert(16, sizeof(PC));

        assert(2 , sizeof(c2));
        assert(24, sizeof(ici2));
    }
#endif

    return 0;
}
