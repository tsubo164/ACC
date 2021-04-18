/*
#include <string.h>
#include <stdlib.h>
*/
#include <stdio.h>

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
    //long reg[7] = {29, 12, 13, 44, 99, 128, 31};
    int reg[7] = {1, 100, 2, 200, 3, 300, 4};
    char *fmt = "(%d, %d, %d, %d)";
    va_list ap;
    char buf[128] = {'\0'};

    ap->gp_offset = 0;
    ap->fp_offset = 48;
    ap->reg_save_area = &reg[0];
    vsprintf(buf, fmt, ap);
    printf("<< %s >>\n", buf);
}
*/

int foo(int a, int b, int c, int d, int e)
{
    return a + b + c + d + e;
}

int bar(int a, int b)//, int c)
{
    return a + b;// + c;
}
int baz(int a)
{
    return a;
}

int main()
{
    //int a = bar(baz(3), 4);
    //int a = bar(baz(3), 4);
    //int b = bar(baz(4), 1);//, 3);
    //int b = bar(4, 1);//, 3);
    //int b = baz(4);
    //bar(baz(0), 0);
    /*
    int a =
    */
        foo(92 + 23 %2, 12 * 3, 4/ 2, 4 % 1, 8 + 3 * 4);
    /*
    foo();
    */
    return 13;
}
