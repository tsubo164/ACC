int num()
{
    int n;
    n = 7;
    return n;
}

int add(int x, int y)
{
    return x + y;
}

int main()
{
    int a;
    a = add(31, 13) +  2 * num();
    /*
    */
    /*
    a = 42;
    */
    a = 6 * num() - 2;
    a = 42 / num();
    /*
    a = 6 != num();
    a = 7 >= num();
    */
    return a;
}
