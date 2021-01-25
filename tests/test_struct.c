int assert(int expected, int actual);

/* struct */
struct point {
    int x;
    int y;
};

int main()
{
    {
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
}
