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

int sum1234_mult_sum5678(
        int a1, int a2, int a3, int a4,
        int a5, int a6, int a7, int a8)
{
    return
        (a1 + a2 + a3 + a4) *
        (a5 + a6 + a7 + a8);
}

int main()
{
    /*
    foo();
    */
    printf("    >> %d\n", sum1234_mult_sum5678(1, 2, 3, 4, 5, 6, 7, 8));
    return 13;
}
