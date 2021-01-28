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
/*
typedef struct vec {
    int x, y, z;
} vec;
struct vec;
*/

//typedef int Integer;
//typedef char CH;

int main()
{
    struct foo {
        int i, j, k;
    } f;
    //Integer i = 42;
    //int a = 13;
    //CH c = 3;
    struct foo ff;

    /*
    struct vec v;
    v.y = 111;
    */

    f.i = 121;
    ff.j = 111;
    //return a;
    //return c + i;
    //return c;
    return f.i - ff.j;
    /*
    return sizeof(vec) ;//+ v.y;
    */
}
