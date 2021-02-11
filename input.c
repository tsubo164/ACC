void puts(char *s);

typedef int id_t;

//extern int exti;
int exti = 13;

static int stai = 19;

static int add(int x, int y)
{
    return x + y;
}

int main()
{
    id_t id = exti;

    id = add(19, 11);
    id = stai;

    puts("Helleo, world!");
    return id;
}
