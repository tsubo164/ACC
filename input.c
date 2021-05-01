/*
#include <stdlib.h>
#include <string.h>
*/
#include <stdio.h>
#include <stdarg.h>

int my_sprintf(char *str, const char *format, ...)
{
    int ret;
    va_list ap;
    va_start(ap, format);
    ret = vsprintf(str, format, ap);
    va_end(ap);
    return ret;
}

void bar(const char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    vprintf(format, ap);
    va_end(ap);
}

int main()
{
    bar("    bar >> %d, %d, %d, %d\n", 11, 22, 33, 44);
    bar("    bar >> %d, %d, %d, %d, %d, %d\n", 11, 22, 33, 44, 55, 66);
    {
        /* my var arg function call with 6 parameters */
        char buf[32] = {'\0'};

        my_sprintf(buf, "(%d, %d, %d, %d, %d, %d)", 1, 2, 3, 4, 55, 66);
        printf("    my_sprintf >> %s\n", buf);
    }
    return 13;
}
