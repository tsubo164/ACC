/*
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
*/

/*
typedef struct {
    unsigned int gp_offset;
    unsigned int fp_offset;
    void *overflow_arg_area;
    void *reg_save_area;
} va_list[1];

void vprintf(const char *format, va_list ap);
void vsprintf(char *str, const char *format, va_list ap);

void foo()
{
    long reg[7] = {29, 12, 13, 44, 99, 128, 31};
    char *fmt = "(%d, %d, %d, %d)";
    va_list ap;
    char buf[128] = {'\0'};

    ap->gp_offset = 0;
    ap->fp_offset = 48;
    //ap.reg_save_area = &reg[0];
    //vprintf(fmt, &ap);
    ap->reg_save_area = &reg[0];
    vsprintf(buf, fmt, ap);
    printf("<< %s >>\n", buf);
}
*/

int bar(int a[1])
{
    return *a;
}

int baz(char a[1][1])
{
    return **a;
}

int main()
{
    int a[1] = {9};
    char *b[] = {"foo"};
    bar(a);
    baz(b);
    //foo();
    //return 13;
    return baz(b);
}
/*
void foo()
{
    long reg[7] = {29, 12, 13, 44, 99, 128, 31};
    char *fmt = "(%d, %d, %d, %d)";
    struct va_list ap;
    char buf[1024] = {'0'};

    ap.gp_offset = 0;
    ap.fp_offset = 48;
    //ap.reg_save_area = &reg[0];
    //vprintf(fmt, &ap);
    ap.reg_save_area = &reg[0];
    vsprintf(buf, fmt, &ap);
    printf("<< %s >>\n", buf);
}
*/
