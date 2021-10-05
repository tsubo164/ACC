#include "test.h"
#include "gcc_func.h"
/*
TODO fix infinite loop. make it enable to include std headers from here
#include "../include/stdio.h"
#include "../include/string.h"
*/
int printf(const char *format, ...);
int sprintf(char *str, const char *format, ...);
int strcmp(const char *s1, const char *s2);

/* function with 8 parameters compiled by acc */
int sum1234_mult_sum5678(
        int a1, int a2, int a3, int a4,
        int a5, int a6, int a7, int a8)
{
    return
        (a1 + a2 + a3 + a4) *
        (a5 + a6 + a7 + a8);
}

typedef struct {
    unsigned int gp_offset;
    unsigned int fp_offset;
    void *overflow_arg_area;
    void *reg_save_area;
} va_list[1];
void __builtin_va_start(va_list ap, void *last);

int vsprintf(char *str, const char *format, va_list ap);

int my_sprintf(char *str, const char *format, ...)
{
    va_list ap;
    __builtin_va_start(ap, &(format));
    return vsprintf(str, format, ap);
}

int main()
{
    {
        assert(42, gcc_twice(21));
        assert(16, gcc_twice(gcc_twice(4)));
    }
    {
        long a = 0;
        gcc_add(11, 22, 33, 44, &a);

        assert(110, a);
    }
    {
        /* function with 8 parameters compiled by gcc */
        int a = gcc_sum1234_mult_sum5678(1, 2, 3, 4, 5, 6, 7, 8);

        assert(260, a);
    }
    {
        /* function with 8 parameters compiled by acc */
        int a = sum1234_mult_sum5678(1, 2, 3, 4, 5, 6, 7, 8);

        assert(260, a);
    }
    {
        /* std var arg function call with 9 parameters */
        char buf[32] = {'\0'};

        sprintf(buf, "(%d, %d, %d, %d, %d, %d, %d)", 1, 2, 3, 4, 5, 6, 7);
        assert(0, strcmp(buf, "(1, 2, 3, 4, 5, 6, 7)"));
    }
    {
        /* my var arg function call with 6 parameters */
        char buf[32] = {'\0'};

        my_sprintf(buf, "(%d, %d, %d, %d)", 1, 2, 3, 4);
        assert(0, strcmp(buf, "(1, 2, 3, 4)"));
    }
    {
        /* my var arg function call with 9 parameters */
        char buf[32] = {'\0'};

        my_sprintf(buf, "(%d, %d, %d, %d, %d, %d, %d)", 1, 2, 3, 4, 5, 6, 7);
        assert(0, strcmp(buf, "(1, 2, 3, 4, 5, 6, 7)"));
    }
    {
        /* gcc 8 byte struct for passing by value */
        point p = {14, 17};

        assert(14, gcc_get_x(p));
        assert(14, p.x);
        assert(17, gcc_get_y(p));
        assert(17, p.y);
    }
    {
        /* gcc 16 byte struct for passing by value */
        vec v = {14, 17, 49, 32};

        assert(14, gcc_get_x4(v));
        assert(14, v.x);
        assert(17, gcc_get_y4(v));
        assert(17, v.y);
        assert(49, gcc_get_z4(v));
        assert(49, v.z);
        assert(32, gcc_get_w4(v));
        assert(32, v.w);
    }
    {
        /* gcc large struct for passing by value */
        Coord c = {111, 222, 199};

        assert(111, gcc_coord_x(c));
        assert(111, c.x);
        assert(222, gcc_coord_y(c));
        assert(222, c.y);
        assert(199, gcc_coord_z(c));
        assert(199, c.z);
    }
    {
        /* gcc 8 byte struct returned by value */
        point p = gcc_get_point();

        assert(71, p.x);
        assert(92, p.y);
    }
    {
        /* gcc 16 byte struct returned by value */
        vec v = gcc_get_vec();

        assert(1301, v.x);
        assert(223922, v.y);
        assert(-3973, v.z);
    }

    return 0;
}
