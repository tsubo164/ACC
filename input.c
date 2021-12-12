/*
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
*/

int main()
{
    const char name[][8] = {"abc", "def"};

    const char *foo = name[0];

    return foo[0];
}
