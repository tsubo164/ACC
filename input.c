//int g_a[3] = {11, 22, 44};

int foo() {
    return 42;
}

enum color {
    R, G, B
};

int main()
{
    int g_a[3] = {11, 22, foo()};
    //int a = G + B;
    //int a = foo();
    /*
    //int a[] = {5, 37, 19, 23, -1};
    int a[3][2] = {{11, 22}, {44, 55}};
    //int a[4] = {11, 22};

    //return sizeof(a);//a[3];
    return a[1][1];
    //return sizeof a;
    */
    return g_a[2];
    //return a;
}
