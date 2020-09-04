int num()
{
    return 7;
}

int main()
{
    int a;
    /*
    int *b;
    */

    a = 35;
    a = a + num();
    /*
    b = &a;

    *b = *b + 2;
    */

    return a;
    /*
    if (a == 40)
        return 0;
    else
        return 1;
    */
}
