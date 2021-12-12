/*
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
*/

#include <stdio.h>

int count = 42;

int main()
{
    int num = 0;
    char buf[8];

    printf("%s%n", "foo", &num);
    printf("%d\n", num);

    sprintf(buf, "%s%n", "foo", &num);
    printf("%d\n", num);

    return num;
}
