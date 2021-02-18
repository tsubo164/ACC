int assert(int expected, int actual);

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

/* multi-dimensional global array initializer with less than specified size */
int g_a[3] = {11, 22, 44};
int g_b[3][2] = {{111, 222}, {444, 555}};

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

    return 0;
}
