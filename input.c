int main()
{
    struct point {
        int x, y, z;
    };

    struct point pt = {11, 22, 33};

    return pt.x;
    /*
    int a[2] = {11, 22};

    return a[0];
    */
}
