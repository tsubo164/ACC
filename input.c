int main()
{
    struct point {
        int x, y, z;
        struct tok {
            int a, b;
        } t;
        int g[2][5];
    };

    struct point pt = {11, 22, 33};

    return pt.z;
        /*
    int a[2][2] = {{11, 22}, {33, 44}};

    return a[0][1];
        */
    /*
    */
}
