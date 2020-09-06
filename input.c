int add(int x, int y)
{
    return x + y;
}

int main()
{
    int a;
    int *b;

    a = 39;
    b = &a;

    *b = *b + 3;

    if (a == 42)
        return 0;
    else
        return 1;
}
