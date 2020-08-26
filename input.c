int main()
{
    int a;
    int b;

    a = 40;
    b = &a;

    *b = *b + 2;

    return *b;
}
