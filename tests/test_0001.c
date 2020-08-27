int main()
{
    int a;
    int *b;

    a = 40;
    b = &a;

    *b = *b + 2;

    if (a == 42)
        return 0;
    else
        return 1;
}
