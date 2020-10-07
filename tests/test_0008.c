/* struct */
struct point {
    int x;
    int y;
};

int main()
{
    struct point p;
    int *ptr;

    p.x = 39;

    ptr = &p.y;
    *ptr = 3; /* p.y = 3; */

    if (p.x + p.y == 42)
        return 0;
    else
        return 1;
}
