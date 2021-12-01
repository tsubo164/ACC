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
#ifdef MORETEST
    {
        /* nested struct */
        typedef struct point {
            int x, y;
        } point;

        struct line {
            point p0, p1;
        };

        struct line l;

        l.p0.x = 19;
        l.p1.y = 23;

        assert(19, l.p0.x);
        assert(23, l.p1.y);
        assert(42, l.p0.x + l.p1.y);
    }
    {
        /* nested struct pointer with typedef */
        typedef struct node Node;

        struct node {
            int id;
            Node *next;
        };

        Node n0, n1, n2;

        n0.id = 123;
        n1.id = 765;
        n2.id = 999;

        n0.next = &n1;
        n1.next = &n2;
        n2.next = 0;

        assert(123, n0.id);
        assert(765, n0.next->id);
        assert(999, n0.next->next->id);
        assert(999, n1.next->id);
    }
    {
        /* nested struct pointer without typedef */
        struct node {
            int id;
            struct node *next;
        };

        struct node n0, n1, n2;

        n0.id = 123;
        n1.id = 765;
        n2.id = 999;

        n0.next = &n1;
        n1.next = &n2;
        n2.next = 0;

        assert(123, n0.id);
        assert(765, n0.next->id);
        assert(999, n0.next->next->id);
        assert(999, n1.next->id);
    }
    {
        /* struct with no tag */
        struct {
            int x, y;
        } no_tag;

        no_tag.x = 111;
        no_tag.y = 19;

        assert(111, no_tag.x);
        assert(19, no_tag.y);
        assert(130, no_tag.x + no_tag.y);
    }
    {
        /* struct initializer */
        struct point {
            int x, y, z;
        } pt = {11, 22, 33};

        assert(11, pt.x);
        assert(22, pt.y);
        assert(33, pt.z);
    }
    {
        /* struct initializer with less members */
        struct point {
            int x, y, z;
        } pt = {19};

        assert(19, pt.x);
        assert(0,  pt.y);
        assert(0,  pt.z);
    }
    {
        /* struct initializer with expressions */
        enum color {
            R = 11,
            G,
            B
        };
        struct point {
            int x, y, z;
        } pt = {19, add(40, 2), R + G + B};

        assert(19, pt.x);
        assert(42, pt.y);
        assert(36, pt.z);
    }
    {
        /* size of struct with internal struct members */
        struct foo {
            int a;
            struct bar {
                int b;
            } b;
        };

        assert(8, sizeof (struct foo));
    }
    {
        /* struct with unsigned int members */
        struct foo {
            const unsigned int i, j;
        } f = {29, 31};

        assert(8, sizeof (struct foo));
        assert(29, f.i);
        assert(31, f.j);
    }
    {
        /* struct pointer for functions */
        struct point p = {42, 71};
        struct point q = {13, 49};
        struct point result;

        /* function calls in function arguments */
        assert(42, getx(&p));
        assert(42, p.x);
        assert(49, gety(&q));

        add_point(&result, &p, &q);

        assert(55, getx(&result));
        assert(120, gety(&result));
    }
    {
        /* initialize struct object with another struct object */
        typedef struct point {
            int x, y, z, w;
        } Point;

        Point p = {11, 22, 33, 44};
        Point q = p;

        assert(11, q.x);
        assert(22, q.y);
        assert(33, q.z);
        assert(44, q.w);
    }
    {
        /* assign struct object */
        typedef struct point {
            int x, y, z, w;
        } Point;

        Point p = {111, 222, 333, 444};
        Point q;

        q = p;

        assert(111, q.x);
        assert(222, q.y);
        assert(333, q.z);
        assert(444, q.w);
    }
    {
        /* 8 byte struct for passing by value */
        struct point p = {14, 17};

        assert(14, getx_(p));
        assert(14, p.x);
        assert(17, gety_(p));
        assert(17, p.y);
    }
    {
        /* 16 byte struct for passing by value */
        vec v = {41, 71, 94};
        vec4 w = {731, 811, 39, 234};

        assert(41, get_x3(v));
        assert(41, v.x);
        assert(71, get_y3(v));
        assert(71, v.y);
        assert(94, get_z3(v));
        assert(94, v.z);

        assert(234, get_w4(w));
        assert(234, w.w);
    }
    {
        /* large struct for passing by value */
        Coord c = {111, 222, 199};

        assert(111, coord_x(c));
        assert(111, c.x);
        assert(222, coord_y(c));
        assert(222, c.y);
        assert(199, coord_z(c));
        assert(199, c.z);
    }
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
