/*
int main();
int *day[7];
char str[10];
char *c;
int *p;
int num()
{
    return 42;
}
*/

struct point {
    int x, y;
} P;

struct point Q;

int main()
{
    struct point pt;
    pt.x = 13;
    pt.y = 29;

    return pt.x + pt.y;
}

