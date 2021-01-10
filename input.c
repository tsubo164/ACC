int printf(char *s, int i);

enum color {
    R, G, B
};

struct point {
    int x, y, z;
};

int main()
{
    struct point pt;
    int a = 42;
    int *p = 0;
    char c = 0;
    int b;

    b = sizeof a;
    printf("size of b: => %d\n", b);

    b = sizeof(a + 2);
    printf("size of b: => %d\n", b);

    b = sizeof p;
    printf("size of b: => %d\n", b);

    b = sizeof *p;
    printf("size of b: => %d\n", b);

    b = sizeof c;
    printf("size of b: => %d\n", b);

    b = sizeof pt;
    printf("size of b: => %d\n", b);

    b = sizeof pt.x;
    printf("size of b: => %d\n", b);

    b = sizeof (struct point );
    printf("size of b: => %d\n", b);

    b = sizeof R;
    printf("size of b: => %d\n", b);

    b = sizeof (enum color);
    printf("size of b: => %d\n", b);

    return b;
}
