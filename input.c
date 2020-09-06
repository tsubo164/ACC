int main()
{
    int a;
    int *b;

    a = 39;
    b = &a;

    *b = 42;
    *b = *b + 2;

    return a;
    /*
    if (a == 42)
        return 0;
    else
        return 1;
    */
}
