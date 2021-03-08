int add(int x, int y);

int main()
{
    struct foo {
        int a;
        struct bar {
            int b;
        } b;
    } f;

    int a = sizeof 2 + 1;
    int b[sizeof(int)] = {0, 1, 2, 3};

    return sizeof (struct foo) + b[3];
}

int add(int x, int y)
{
    return x + y;
}
