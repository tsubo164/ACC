int assert(int expected, int actual);

int main()
{
    {
        int a;
        int *b;

        a = 40;
        b = &a;

        *b = *b + 2;

        assert(42, a);
    }
    {
        int a[4];
        int *p = a;

        a[0] = 11;
        a[1] = 22;
        a[2] = 33;
        a[3] = 44;

        assert(11, *p++);
        assert(22, *p);

        p++;
        assert(33, *p);
        assert(33, *p++);
        assert(44, *p);
    }
    {
        /* TODO char -> int cast */
        /*
        char a[4];
        char *p = a;

        a[0] = 11;
        a[1] = 22;
        a[2] = 33;
        a[3] = 44;

        assert(11, *p++);
        assert(22, *p);

        p++;
        assert(33, *p);
        assert(33, *p++);
        assert(44, *p);
        */
    }

    return 0;
}
