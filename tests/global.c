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

/* initialize array of array of pointer to char */
const char *SI__[] = {"sil", "si", "esi", "rsi"};
const char *DI__[] = {"dil", "di", "edi", "rdi"};
/* initialize static array of array of pointer to char */
static const char *C__[]  = {"cl",  "cx", "ecx", "rcx"};
static const char *D__[]  = {"dl",  "dx", "edx", "rdx"};
const char **ARG_REG__[] = {DI__, SI__, D__, C__};

/* array initializer with string literal */
char first_name[] = "Foo";
char last_name[8] = "Bar";

/* array of string initializer with string literal */
char color_list[][10] = {
    "Red",
    "Green",
    "Blue"
};

/* array 7 of string initializer with string literal */
char days[7][4] = { "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun" };

/* initialize bit fields */
struct bitfield1 {
    signed int a : 13;
    signed int b : 17;
} bf1 = {-73, 991};

struct bitfield2 {
    signed int a : 11;
    signed int : 0;
    signed int b : 19;
} bf2 = {-803, -73331};

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
    {
        /* initialize array of array of pointer to char */
        assert(32, sizeof ARG_REG__);
        assert(0, strcmp("dil", ARG_REG__[0][0]));
        assert(0, strcmp("di",  ARG_REG__[0][1]));
        assert(0, strcmp("edi", ARG_REG__[0][2]));
        assert(0, strcmp("rdi", ARG_REG__[0][3]));
        assert(0, strcmp("sil", ARG_REG__[1][0]));
        assert(0, strcmp("si",  ARG_REG__[1][1]));
        assert(0, strcmp("esi", ARG_REG__[1][2]));
        assert(0, strcmp("rsi", ARG_REG__[1][3]));
        /* initialize static array of array of pointer to char */
        assert(0, strcmp("dl",  ARG_REG__[2][0]));
        assert(0, strcmp("dx",  ARG_REG__[2][1]));
        assert(0, strcmp("edx", ARG_REG__[2][2]));
        assert(0, strcmp("rdx", ARG_REG__[2][3]));
        assert(0, strcmp("cl",  ARG_REG__[3][0]));
        assert(0, strcmp("cx",  ARG_REG__[3][1]));
        assert(0, strcmp("ecx", ARG_REG__[3][2]));
        assert(0, strcmp("rcx", ARG_REG__[3][3]));
    }
    {
        /* array initializer with string literal */
        assert(4, sizeof first_name);

        assert('F', first_name[0]);
        assert('o', first_name[1]);
        assert('o', first_name[2]);
        assert('\0', first_name[3]);
    }
    {
        /* array initializer with string literal and specified length */
        assert(8, sizeof last_name);

        assert('B', last_name[0]);
        assert('a', last_name[1]);
        assert('r', last_name[2]);
        assert('\0', last_name[3]);
        assert('\0', last_name[4]);
        assert('\0', last_name[5]);
        assert('\0', last_name[6]);
        assert('\0', last_name[7]);
    }
    {
        /* array of string initializer with string literal */
        assert(30, sizeof color_list);

        assert('R',  color_list[0][0]);
        assert('e',  color_list[0][1]);
        assert('d',  color_list[0][2]);
        assert('\0', color_list[0][3]);
        assert('\0', color_list[0][4]);
        assert('\0', color_list[0][5]);
        assert('\0', color_list[0][6]);
        assert('\0', color_list[0][7]);
        assert('\0', color_list[0][8]);
        assert('\0', color_list[0][9]);

        assert('G',  color_list[1][0]);
        assert('r',  color_list[1][1]);
        assert('e',  color_list[1][2]);
        assert('e',  color_list[1][3]);
        assert('n',  color_list[1][4]);
        assert('\0', color_list[1][5]);
        assert('\0', color_list[1][6]);
        assert('\0', color_list[1][7]);
        assert('\0', color_list[1][8]);
        assert('\0', color_list[1][9]);

        assert('B',  color_list[2][0]);
        assert('l',  color_list[2][1]);
        assert('u',  color_list[2][2]);
        assert('e',  color_list[2][3]);
        assert('\0', color_list[2][4]);
        assert('\0', color_list[2][5]);
        assert('\0', color_list[2][6]);
        assert('\0', color_list[2][7]);
        assert('\0', color_list[2][8]);
        assert('\0', color_list[2][9]);
    }
    {
        /* array 7 of string initializer with string literal */
        assert(28, sizeof days);

        assert('M',  days[0][0]);
        assert('o',  days[0][1]);
        assert('n',  days[0][2]);
        assert('\0', days[0][3]);

        assert('T',  days[1][0]);
        assert('u',  days[1][1]);
        assert('e',  days[1][2]);
        assert('\0', days[1][3]);

        assert('W',  days[2][0]);
        assert('e',  days[2][1]);
        assert('d',  days[2][2]);
        assert('\0', days[2][3]);

        assert('T',  days[3][0]);
        assert('h',  days[3][1]);
        assert('u',  days[3][2]);
        assert('\0', days[3][3]);

        assert('F',  days[4][0]);
        assert('r',  days[4][1]);
        assert('i',  days[4][2]);
        assert('\0', days[4][3]);

        assert('S',  days[5][0]);
        assert('a',  days[5][1]);
        assert('t',  days[5][2]);
        assert('\0', days[5][3]);

        assert('S',  days[6][0]);
        assert('u',  days[6][1]);
        assert('n',  days[6][2]);
        assert('\0', days[6][3]);
    }
    {
        /* initialize bit fields */
        assert(4, sizeof bf1);
        assert(8, sizeof bf2);

        assert(-73, bf1.a);
        assert(991, bf1.b);

        assert(-803, bf2.a);
        assert(-73331, bf2.b);
    }

    return 0;
}
