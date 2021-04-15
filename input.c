/*
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
*/

typedef struct Point {
    int x, y;
} Point;

int main()
{
    typedef int Array[3];
    typedef int Integer;
    typedef Integer ID;
    typedef int *Ref;
    Integer i = 32;
    Array a = {11, 22, 33};
    Point p = {99, 88};
    Ref r = &i;

    return a[2] + p.y + i + *r;
}

/*
struct va_list {
    int gp_offset;
    int fp_offset;
    void *overflow_arg_area;
    void *reg_save_area;
};
void vprintf(const char *fmt, struct va_list *ap);
void vsprintf(char *buf, const char *fmt, struct va_list *ap);

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

int main()
{
    foo();
    return 13;
}
*/
