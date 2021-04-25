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

int vprintf(const char *format, va_list ap);
int vsprintf(char *str, const char *format, va_list ap);

void bar(const char *fmt, ...)
{
    va_list ap;
    ap->gp_offset = 8;
    ap->fp_offset = 48;
    ap->reg_save_area = &fmt;
    vprintf(fmt, ap);
}

int main()
{
    bar("    >> %d, %d, %d, %d\n", 11, 22, 33, 44);
    return 13;
}
