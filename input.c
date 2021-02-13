int main()
{
    int a[2] = {5, 37, "foo"};
    //int a[2] = {5, 37, 19};
    int i = 111;

    a[1] = 19;

    return a[1] + i;
}
