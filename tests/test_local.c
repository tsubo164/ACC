int assert(int expected, int actual);

// testing nested block and scope rules for symbol lookup

int add(int x, int y)
{
    return x + y;
}

enum color {
    R = 10, G, B
};

int main()
{
    {
        int x; // this 'x' is different from parameter for add()

        x = add(40, 2);

        assert(42, x);

        {
            int a;
            a = 1;
            x = x + a;

            assert(43, x);
        }
        {
            /* this is different 'a' than one above */
            int a;
            a = 1;
            x = x + a;

            assert(44, x);
        }
        {
            {
                int b;
                b = 1;
                x = x + b;

                assert(45, x);
            }
        }
        {
            {
                {
                    /*
                     * this block can't see 'b' above even though
                     * nested level is shallower than * here
                     */
                    int b;
                    b = 1;
                    x = x + b;

                    assert(46, x);
                }
            }
        }

        assert(46, x);
    }
    {
        /* initialize wity expressions */
        int a[3] = {R + G + B, 22, add(G * 2, 13)};
        int b = add(2, 3) * B;

        assert(33, a[0]);
        assert(22, a[1]);
        assert(35, a[2]);

        assert(60, b);
    }

    return 0;
} // testing line comment at the last line
