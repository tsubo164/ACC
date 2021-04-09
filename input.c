/*
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
*/

/*
char s[3] = "abc";
char a[3] = {11, 22, 33};

char *str = "abc";
*/

int i = 42;// + 5;
int *p = &i;

/*
struct foo {
    int i;
    char s[8];
} f = {42, "FOO"};
*/

int main()
{
    /*
    char *hoge = "gef";
    int a = 19;
    short s[] = {11, 22};
    */

    //return f.i;
    //return hoge[1];
    //return str[1];
    return *p;
}
