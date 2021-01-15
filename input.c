int printf(char *s, int i);

struct point
/*
{
    int x, y;
}
*/
;

struct vec {
    int x, y, z;
};

int main()
{
    //struct point p;
    struct vec v;
    //void k;

    v.x = 42;

    //p.x = 13;

    return v.x;
}
