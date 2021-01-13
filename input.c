int printf(char *s, int i);

struct vec {
    int x, y, z;
};

int main()
{
    struct vec v;
    int i = 118;

    v.x = i;

    return v.x;
}
