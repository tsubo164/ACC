#include "test.h"

int strcmp(const char *s1, const char *s2);

/* initialization for local and global variables */
int X = 8;
char Y;

int num()
{
    /* returns 8 */
    return X;
}

int add(int X, int Y)
{
    return X + Y;
}

struct point {
    int x, y, z;
} g_pt = {11, 59, 91};

struct point g_line[2] = {{11, 59, 91}, {77}};

struct name {
    int first[4];
    int last[4];
} g_someone = {{1, 2}, {11, 22}};

int x;
int z;

/* defined in test.c */
extern int g_count;
void set_count(int val);

/* static local variable */
static int count_up()
{
    static int count = 0;
    return ++count;
}

/* static local variable with the same name */
static int count_up2()
{
    static int count = 0;
    return ++count;
}

/* initialize with pointer to global variable */
int ival = 42;
int *pval = &ival;

/* multi-dimensional global array initializer with less than specified size */
int g_a[3] = {11, 22, 44};
int g_b[3][2] = {{111, 222}, {444, 555}};

/* initialize global variable with enumerator and cast expressions */
#define NULL ((void *)0)
enum {
    SMALL,
    MEDIUM,
    LARGE
};
struct list {
    int id;
    const char *next;
} head = {MEDIUM, NULL};

/* initialize array of struct */
typedef struct coord {
    long x, y, z;
} Coord;

Coord verts[] = {
    {111, 222, 333},
    {444, 555, 666},
    {777, 888, 999}
};

/* initialize array of struct that has pointer to char */
struct data_spec {
    const char *suffix;
    const char *sizename;
};

const struct data_spec data_spec_table[] = {
    {"b", "byte"},
    {"w", "word"},
    {"l", "long"},
    {"q", "quad"}
};

/* initialize array of to char */
const char *A__[]  = {"al",  "ax", "eax", "rax"};

/* initialize to char */
char *str = "Hello";
int *p = ((void*)1);

int main()
{
    {
        char a = 31;
        int b = 3 + a;

        assert(34, b);
        assert(42, b + num() + Y);
    }
    {
        /* global variables and pointer */
        int a;
        int *p;

        p = &x;

        *p = 3;
        assert(3, x);

        a = add(39, x);
        assert(42, a);

        z = a;
        assert(42, z);

        assert(45, z + *p);
    }
    {
        /* external global variables */
        g_count = 212;
        assert(212, g_count);

        set_count(19);
        assert(19, g_count);
    }
    {
        /* static local variable */
        assert(1, count_up());
        assert(2, count_up());
        assert(3, count_up());
        assert(4, count_up());
        assert(5, count_up());
        assert(6, count_up());
    }
    {
        /* static local variable with the same name */
        assert(1, count_up2());
        assert(2, count_up2());
        assert(3, count_up2());
        assert(4, count_up2());
        assert(5, count_up2());
        assert(6, count_up2());
    }
    {
        /* global array initializer */
        assert(12, sizeof g_a);
        assert(11, g_a[0]);
        assert(22, g_a[1]);
        assert(44, g_a[2]);
    }
    {
        /* multi-dimensional global array initializer with less than specified size */
        assert(24, sizeof g_b);
        assert(111, g_b[0][0]);
        assert(222, g_b[0][1]);
        assert(444, g_b[1][0]);
        assert(555, g_b[1][1]);
        assert(  0, g_b[2][0]);
        assert(  0, g_b[2][1]);
    }
    {
        /* global struct initializer */
        assert(12, sizeof g_pt);
        assert(11, g_pt.x);
        assert(59, g_pt.y);
        assert(91, g_pt.z);
    }
    {
        /* global initializer for array of struct */
        assert(24, sizeof g_line);
        assert(11, g_line[0].x);
        assert(59, g_line[0].y);
        assert(91, g_line[0].z);
        assert(77, g_line[1].x);
        assert( 0, g_line[1].y);
        assert( 0, g_line[1].z);
    }
    {
        /* global initializer for struct with array members */
        assert(32, sizeof g_someone);
        assert(1, g_someone.first[0]);
        assert(2, g_someone.first[1]);
        assert(0, g_someone.first[2]);
        assert(0, g_someone.first[3]);
        assert(11, g_someone.last[0]);
        assert(22, g_someone.last[1]);
        assert( 0, g_someone.last[2]);
        assert( 0, g_someone.last[3]);
    }
    {
        /* initialize with pointer to global variable */
        assert(4, sizeof ival);
        assert(8, sizeof pval);
        assert(42, *pval);
    }
    {
        /* initialize global variable with enumerator and cast expressions */
        assert(MEDIUM, head.id);
        assertl(0, (long)head.next);
    }
    {
        /* initialize array of struct */
        Coord c;
        assert(72, sizeof verts);
        assert(111, verts[0].x);
        assert(222, verts[0].y);
        assert(333, verts[0].z);
        assert(444, verts[1].x);
        assert(555, verts[1].y);
        assert(666, verts[1].z);
        assert(777, verts[2].x);
        assert(888, verts[2].y);
        assert(999, verts[2].z);

        c = verts[2];
        assert(777, c.x);
        assert(888, c.y);
        assert(999, c.z);
    }
    {
        /* initialize array of struct that has pointer to char */
        assert(64, sizeof data_spec_table);
        assert(0, strcmp("b",    data_spec_table[0].suffix));
        assert(0, strcmp("byte", data_spec_table[0].sizename));
        assert(0, strcmp("w",    data_spec_table[1].suffix));
        assert(0, strcmp("word", data_spec_table[1].sizename));
        assert(0, strcmp("l",    data_spec_table[2].suffix));
        assert(0, strcmp("long", data_spec_table[2].sizename));
        assert(0, strcmp("q",    data_spec_table[3].suffix));
        assert(0, strcmp("quad", data_spec_table[3].sizename));
    }
    {
        /* initialize array of to char */
        assert(32, sizeof A__);
        assert(0, strcmp("al",  A__[0]));
        assert(0, strcmp("ax",  A__[1]));
        assert(0, strcmp("eax", A__[2]));
        assert(0, strcmp("rax", A__[3]));
    }
    {
        /* initialize to char */
        assert(8, sizeof str);
        assert(0, strcmp("Hello",  str));
        assert(1, (int) p);
    }

    return 0;
}
