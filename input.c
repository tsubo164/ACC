int foo() {
    return 42;
}

int main()
{
    struct point {
        int x, y, z;
    } pt = {11, foo()};

    return pt.y;
    /*
    int a[2][2] = {{11, 22}, {33, 44}};

    return a[0][1];
    */
    /*
    int a[3] = {11};

    return a[0];
    */
}
