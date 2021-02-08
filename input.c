void puts(char *s);

int main()
{
    int x[4];

    int a = 5;
    int b = 10;

    if (a > b)
        goto final;

final:
    puts("Hello, world!\n");

    return a;
}
