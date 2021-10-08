#include "test.h"

/* struct */
struct point {
    int x;
    int y;
};

int getx(struct point *p)
{
    return p->x;
}

int gety(struct point *p)
{
    return p->y;
}

int getx_(struct point p)
{
    return p.x;
}

int gety_(struct point p)
{
    return p.y;
}

void add_point(struct point *out, struct point *p, struct point *q)
{
    out->x = p->x + q->x;
    out->y = p->y + q->y;
}

struct point get_point()
{
    struct point p = {71, 92};
    return p;
}

typedef struct vec {
    int x, y, z;
} vec;

int add(int x, int y) {
    return x + y;
}

int get_x3(vec v)
{
    return v.x;
}

int get_y3(vec v)
{
    return v.y;
}

int get_z3(vec v)
{
    return v.z;
}

vec get_vec()
{
    vec v = {1301, 223922, -3973};
    return v;
}

void copy_vec(vec *dst, const vec *src)
{
    if (!dst || !src || dst == src)
        return;
    *dst = *src;
}

typedef struct vec4 {
    int x, y, z, w;
} vec4;

int get_w4(vec4 v)
{
    return v.w;
}

typedef struct coord {
    long x, y, z;
} Coord;

long coord_x(Coord c)
{
    return c.x;
}

long coord_y(Coord c)
{
    return c.y;
}

long coord_z(Coord c)
{
    return c.z;
}

Coord get_coord(void)
{
    Coord c = {72340, -1230889, 91355};
    return c;
}

int main()
{
    {
        /* struct and pointer */
        struct point p;
        int *ptr;

        p.x = 39;
        assert(39, p.x);

        ptr = &p.y;
        *ptr = 3;
        assert(3, p.y);

        assert(42, p.x + p.y);
    }
    {
        /* pointer access of struct */
        struct point p;
        struct point *pp;

        pp = &p;

        pp->x = 9;
        assert(9, p.x);

        pp->y = 3;
        assert(3, p.y);

        assert(12, pp->x + pp->y);

        assert(8, sizeof pp);
        assert(8, sizeof p);
        assert(8, sizeof *pp);
        assert(4, sizeof pp->x);
        assert(8, sizeof (struct point));
    }
    {
        /* array of struct */
        struct foo {
            int i, j, k;
        } f;

        struct foo ff;
        struct foo fa[3];

        f.i = 121;
        ff.j = 111;

        assert(121, f.i);
        assert(111, ff.j);
        assert(10, f.i - ff.j);
        assert(12, sizeof(struct foo));

        fa[1].j = 142;
        fa[2].k = 42;
        assert(142, fa[1].j);
        assert(42, fa[2].k);
    }
    {
        /* typedef'ed struct */
        vec v0, v1;
        vec *vp = &v1;

        v0.x = 123;
        v1.y = 23;

        assert(123, v0.x);
        assert(23, v1.y);

        vp->x = 31;
        vp->z = 91;

        assert(31, v1.x);
        assert(91, v1.z);
    }
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

    return 0;
}
