int main()
{
    int a[4];

    *(a + 1 + 1) = 42;

    if (*(a + 2) == 42)
        return 0;
    else
        return 1;
}
