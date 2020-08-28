int add(int x, int y)
{
    return x + y;
}

int main()
{
    int x;

    x = add(40, 2);

    {
        int a;
        a = 1;
        x = x + a;
    }
    {
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
}
