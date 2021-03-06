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

/* testing stack alignment for nested function call */
int foo(int a, int b, int c)
{
    return a + b + c;
}
/* testing stack alignment for nested function call */
int bar(int a)
{
    return 2 * a;
}

Coord make_coord(long l)
{
    Coord c = {7, -19, 0};
    c.z = l;
    return c;
}

long some_calc(long a, long b, Coord coord)
{
    return a + b + coord.x;
}

void some_coord_func(const struct coord *c, int *nchars)
{
    *nchars = 3;
}

/* calling function returning large struct in a function that has no local area */
long try_some_calc(void)
{
    return some_calc(19, 27, make_coord(55));
}

/* mid size struct param to be stored correctly without stepping on rdx */
int mid_struct_and_some_ptr(const char *s, int *i, vec p)
{
    return *i + p.x;
}

/* testing sign extension on parameter passing */
long twice_long(long l)
{
    return 2 * l;
}

/* returning struct by value with a parameter using rdi */
Coord ret_struct_by_val(const char *s, int id)
{
    Coord c = {11, 22, 33};
    c.x += id;
    c.y += id;
    c.z += id;
    return c;
}

/* returning struct by value with a parameter using rdi and taking struc value param */
Coord ret_struct_by_val_with_struct_param(Coord cd, const char *s, int id)
{
    Coord c = cd;
    c.x += id;
    c.y += id;
    c.z += id;
    return c;
}

/* function pointer */
int num()
{
    return 23012;
}

/* global variables of function pointer */
int (*function_pointer)();
static int (*function_pointer2)() = num;
long (*function_pointer3)(long);
static long (*function_pointer4)(long) = twice_long;

/* typedef function pointer */
typedef int (*fn_type)() = num;
typedef long (*fn_type2)(long) = twice_long;

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
    {
        /* gcc large struct returned by value */
        Coord c = gcc_get_coord();

        assert(72340, c.x);
        assert(-1230889, c.y);
        assert(91355, c.z);
    }
    {
        /* testing stack alignment for nested function call */
        int a;

        a = foo(11, 22, bar(33));

        assert(99, a);
    }
    {
        /* large struct passed by value in nested function call */
        long l = 23;

        l = some_calc(9, 17, make_coord(55));

        assert(33, l);
    }
    {
        Coord c = {111, 222, 333};
        int a = 17;

        some_coord_func(&c, &a);

        assert(3, a);
    }
    {
        /* calling function returning large struct in a function that has no local area */
        long l = try_some_calc();

        assert(53, l);
    }
    {
        /* mid size struct param to be stored correctly without stepping on rdx */
        vec p = {11, 22, 33};
        const char *s = "Heloo, world\n";
        int i = 42;

        assert(53, mid_struct_and_some_ptr(s, &i, p));
    }
    {
        /* testing sign extension on parameter passing */
        long l;
        int i = -32;
        char buf[32] = {'\0'};

        l = twice_long(i);
        sprintf(buf, "%ld", l);

        /* need compare string, otherwise equality will succeed with the wrong value */
        assert(0, strcmp("-64", buf));
    }
    {
        /* returning struct by value with a parameter using rdi */
        Coord c = ret_struct_by_val("foo", 19);

        assert(30, c.x);
        assert(41, c.y);
        assert(52, c.z);
    }
    {
        Coord c = {111, 222, 333};

        c = ret_struct_by_val_with_struct_param(c, "foo", 23);

        assert(134, c.x);
        assert(245, c.y);
        assert(356, c.z);
    }
    {
        /* function pointer */
        int (*fp)() = num;
        int (*fp2)();

        assert(8, sizeof fp);
        assert(23012, fp());

        fp2 = num;

        assert(8, sizeof fp2);
        assert(23012, fp2());
    }
    {
        /* function pointer with parameters */
        long (*fp)(long) = twice_long;
        long (*fp2)(long);

        assert(8, sizeof fp);
        assertl(246, fp(123));

        fp2 = twice_long;

        assert(8, sizeof fp2);
        assertl(1998, fp2(999));
    }
    {
        /* global variable of function pointer */
        function_pointer = num;

        assert(8, sizeof function_pointer);
        assert(23012, function_pointer());

        assert(8, sizeof function_pointer2);
        assert(23012, function_pointer2());
    }
    {
        /* global variable of function pointer with parameters */
        function_pointer3 = twice_long;

        assert(8, sizeof function_pointer3);
        assert(982, function_pointer3(491));

        assert(8, sizeof function_pointer4);
        assertl(-306, function_pointer4(-153));
    }
    {
        /* typedef function pointer */
        fn_type fp = num;
        fn_type2 fp2 = twice_long;

        assert(8, sizeof(fn_type));
        assert(8, sizeof(fn_type2));

        assert(8, sizeof fp);
        assert(23012, fp());

        assert(8, sizeof fp2);
        assertl(462, fp2(231));
        assertl(-1422, fp2(-711));
    }

    return 0;
}
