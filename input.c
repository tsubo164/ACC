int printf(char *s, int i);

struct vec {
    int x, y, z;
};

int main()
{
    struct vec v;

    v.x = 19;

    return v.x;
}
