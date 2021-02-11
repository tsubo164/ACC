void puts(char *s);

typedef int id_t;

//extern int exti;
int exti = 13;

int main()
{
    id_t id = exti;

    puts("Helleo, world!");
    return id;
}
