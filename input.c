/*
int foo() {
    return 42;
}
struct point {
    int x;
    int y;
    int z;
} pt[2] = {{11, 59, 91}, {1, 2, 3}};
*/

struct name {
    int first[4];
    int last[4];
} me = {{1, 2}, {11, 22}};

/*
struct point {
    int x, y;
    int z;
} pt = {11, 59, 91};

int a[3] = {11, 99, 91};
*/
//enum { R, G, B};
/*
static int b[2][2] = {{11, 22}, {33, 44}};
int x = 99;
int y;
*/

int main()
{
    return me.last[1];
    /*
    return pt[0].z;
    int a = 13;
    return a + x;
        int a[3] = {11, 99, 34};
        int b[3] = {11, 99, 34};

        return b[1][1];
        return a[2];
    int c = 'x';
    int a[2][2] = {{11, 22}, {33, 44}};
    */

    /*
    return a[1][0];

    return pt.y;
    */
    /*
    int *p = 0, i = 0;

    return i;
    struct point {
        int x, y, z;
        struct foo {
            int a, b;
        } f;
    } pt = {{11, 12, 13}, {1, 2}};
    */
    /*
    int a = 123;
    return x;
    */
        /*
    struct point {
        //int x, y, z;
        int x;
        int y;
        int z;
    } pt = {11, 12, 14};

    return pt.z;
        */

    /*
    struct point {
        int x, y, z;
        int a[8];
    } pt = {11, foo()};

    int a[2][3] = {{0, 1, 2}, {3, 4, 5}};

    return pt.y + a[0][0];
    int a[][2] = {{11, 22}, {33, 44}};

    return a[0][1];
    */
    /*
    int a[] = {15, 37, 19, 23};
    */
    /*
    int a[3] = {111, 29, 17};
    return a[0];
    struct point {
        int x, y;
    } line[3] = {{1, 2}, {3, 4}, {5, 6}};

    return line[0].y;
    */
}
