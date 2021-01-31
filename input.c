/*
typedef struct vec {
    int x, y, z;
} vec;
*/

int main()
{
    int a[4];
    int *p = a;

    a[0] = 11;
    a[1] = 22;
    a[2] = 33;
    a[3] = 44;
    /*
    */
    p++;
    p++;
    p++;
    /*
    char a[4];
    char *p = a;

    a[0] = 11;
    a[1] = 22;
    a[2] = 33;
    a[3] = 44;
    p++;
    p++;
    p++;
    */

    return --*p;
    /*
    return *p;
    return 0;
    return 0;
    */
}
