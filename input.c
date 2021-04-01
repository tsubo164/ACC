//#include <stddef.h>

int main()
{
    //typedef int ID;
    static const struct foo {
        int a;
    } f;

    /*
    ID i = 19;

    return i;
    */
    f.a = 19;
    return f.a;
}
