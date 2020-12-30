int printf(char *s, int i);

int main()
{
    int a = 0;
    int b = 7;
    int c;

    c = !(a || b++);

    return c;
}
