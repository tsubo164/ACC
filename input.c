int printf(char *s, int i);

int main()
{
    int a = 2;
    int b;

    b = a < 10 ? 111 : a > 50 ? 1111 : 19;

    printf("b: %d\n", b);

    return 0;
}
