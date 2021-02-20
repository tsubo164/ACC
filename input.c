int foo() {
    return 42;
}

int main()
{
    struct point {
        int x, y, z;
        struct foo {
            int a, b;
        } f;
    } pt = {{11, 12, 13}, {1, 2}};

    return pt.x;
    /*
    */

    /*
    struct point {
        int x, y, z;
        int a[8];
    } pt = {11, foo()};

    int a[2][3] = {{0, 1, 2}, {3, 4, 5}};

    return pt.y + a[0][0];
    int a[2][2] = {{11, 22}, {33, 44}};

    return a[0][1];
    */
    /*
    int a[3] = {111, 29, 17};

    return a[0];
    */
}
