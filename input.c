#include <stdio.h>
#include <string.h>

int main()
{
    //typedef int ID;
    static const struct foo {
        int a;
    } f;

    char *s = "hello, world!\n";
    char *t = "aello, world!\n";
    /*
    ID i = 19;
    return i;
    */
    int i = strcmp(s, t);
    printf("---------> %d\n", i);
    //f.a = 19;
    return strcmp(s, t);
}
