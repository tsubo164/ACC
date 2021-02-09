int assert(int expected, int actual);

/* struct */
struct point {
    int x;
    int y;
};

typedef struct vec {
    int x, y, z;
} vec;

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
        struct {
            int x, y;
        } no_tag;

        no_tag.x = 111;
        no_tag.y = 19;

        assert(111, no_tag.x);
        assert(19, no_tag.y);
        assert(130, no_tag.x + no_tag.y);
    }

    return 0;
}
