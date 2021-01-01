int assert(int expected, int actual);

void foo()
{
    int a = 1;

    if (a > 0)
        return;

    return;
}

int main()
{
    {
        /* char data type */
        char x[2];
        int a;

        x[0] = 2;
        assert(2, x[0]);

        *(x + 1) = -7;
        assert(-7, x[1]);

        x[0] = x[1] - x[0];
        assert(-9, x[0]);

        a = 51;
        assert(42, a + x[0]);
    }
    {
        /*
        void i = 0;
        */
    }

    return 0;
}
