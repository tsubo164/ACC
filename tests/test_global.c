int assert(int expected, int actual);

int add(int x, int y)
{
    return x + y;
}

int x;
int z;

extern int g_count;
void set_count(int val);

int main()
{
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

    return 0;
}
