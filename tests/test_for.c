int assert(int expected, int actual);

int main()
{
    {
        int sum = 0;
        int i;

        for (i = 1; i <= 10; i++) {
            sum = sum + i;
        }
        assert(55, sum);
    }

    {
        int i;

        for (i = 0; i < 10; ) {
            i = 2 * i + 1;
        }

        assert(15, i);
    }

    {
        int sum = 0;
        int i = 11;

        for (; i <= 20; ) {
            sum = sum + i;
            i++;
        }

        assert(155, sum);
    }

    /*
    for (;;) {}
    */

    return 0;
}
