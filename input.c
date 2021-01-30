/* array index access */
    /*
int main()
{
    int a[4];

    a[0] = 3;
    a[1] = 7;

    a[2] = a[0] + a[1];  // a[2] = 20;
    a[3] = a[2] * 4 + 2; // a[3] = 42;

    if (*(a+3) == 42)
        return 0;
    else
        return 1;
    return 0;
}
    */
typedef struct vec {
    int x, y, z;
} vec;
/*
struct vec;
struct vec {
    int x, y, z;
};
*/

typedef int Integer;
typedef Integer myInt;
typedef myInt Integer;
//typedef char CH;

int main()
{
    vec v;
    struct foo {
        int i, j, k;
    } f;
    /*
    */
    myInt mi = 3;
    Integer i;
    //int a = 13;
    //CH c = 3;
    //struct foo ff;
    //v.x = 12;
    i = 42;
    i = mi;

    /*
    struct vec v;
    v.y = 111;
    */

    f.i = 121;

    v.x = 39;
    /*
    ff.j = 111;
    */
    //return a;
    //return c + i;
    //return c;
    //return i;
    return f.i + v.x;
    /*
    return sizeof(vec) ;//+ v.y;
    */
}
