int printf(char *s, int i);

void foo()
{
    int a = 1;

    if (a > 0)
        return;

    printf("a: %d\n", a);
    return;
}

int main()
{
    //void a = 11;
    int a = 11;

    foo();

    return a;
}
