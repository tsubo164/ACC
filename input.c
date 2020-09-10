int main()
{
    int a[4];

    a[0] = 3;
    a[1] = 7;

    a[2] = a[0] + a[1];  // a[2] = 20;
    a[3] = a[2] * 4 + 2; // a[3] = 42;

    if (*(a+3) == 42)
        return 0;
    else
        return 1;
}
