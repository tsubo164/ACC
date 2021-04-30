/*
#include <string.h>
#include <stdlib.h>
*/
#include <stdio.h>
#include <string.h>

typedef struct {
    unsigned int gp_offset;
    unsigned int fp_offset;
    void *overflow_arg_area;
    void *reg_save_area;
} va_list[1];
void __builtin_va_start(va_list ap, void *last);

int vprintf(const char *format, va_list ap);
int vsprintf(char *str, const char *format, va_list ap);

int my_sprintf(char *str, const char *format, ...)
{
    va_list ap;
    __builtin_va_start(ap, &(format));
    return vsprintf(str, format, ap);
}

void bar(const char *format, ...)
{
    va_list ap;
    __builtin_va_start(ap, &(format));
    vprintf(format, ap);
}

#define ADD(aa, bb) ((aa)+(bb))
int x = ADD(2, 3);
#define SQUARE(x) ((x) * (x))
int y = 4;
int z = SQUARE(y++);

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
