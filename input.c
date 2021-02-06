typedef struct vec {
    int x, y;
} vec;

int main()
{
    struct vec a[3][2];

    a[2][1].x = 13;
    a[2][1].y = 31;

    return a[2][1].y;
}
