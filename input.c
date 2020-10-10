struct point {
    int x;
    int y;
};

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
