/*
int x;
char x;
*/
/*
int *day[7];
*/

struct point {
    int x;
    int y;
};

char glbl = 11;

int twice(int parm)
{
    return 2 * parm;
}

int add(int x, int y)
{
    return x + y;
}

int main()
{
    struct point pt;
    int local;

    pt.x = 11;
    local = -8;

    return twice(glbl + local) + add(pt.x, 25);
}

/*
int main()
{
    int a;
    int *p;
    char c = 42;
    int d[3];

    struct point pt;

    d[1] = c;

    p = &a;
    *p = c;

    *p = d[1];

    pt.x = 8;

    return a + pt.x;
}
*/
