// testing nested block and scope rules for symbol lookup

int add(int x, int y)
{
    return x + y;
}

int main()
{
    int x; // this 'x' is different from parameter for add()

    x = add(40, 2);

    {
        int a;
        a = 1;
        x = x + a;
    }
    {
        /* this is different 'a' than one above */
        int a;
        a = 1;
        x = x + a;
    }
    {
        {
            int b;
            b = 1;
            x = x + b;
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
            }
        }
    }

    if (x == 42 + 4)
        return 0;
    else
        return 1;
} // testing line comment at the last line
