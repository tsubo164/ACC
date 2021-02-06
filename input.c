typedef struct point {
    int x, y;
} point;

struct line {
    point p0, p1;
};

int main()
{
    struct line l;

    l.p0.x = 19;

    return l.p0.x;
}
