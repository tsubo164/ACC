int add(int x, int y);

int main()
{
    struct foo {
        int a;
        struct bar {
            int b;
        } b;
    } f;

    return sizeof (struct foo);
}

int add(int x, int y)
{
    return x + y;
}
