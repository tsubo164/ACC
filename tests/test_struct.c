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
}
